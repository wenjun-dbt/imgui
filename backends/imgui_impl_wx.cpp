
#include "imgui_impl_wx.h"

#include <chrono>

#include "wx/window.h"
#include "wx/clipbrd.h"
#include "wx/cursor.h"

const char* ImGui_ImplWX_GetClipboardText(void* user_data);
void ImGui_ImplWX_SetClipboardText(void* user_data, const char* text);


struct ImGui_ImplWX_Data
{
	std::string clipboardText;
	ImGuiMouseCursor lastMouseCursor;
	wxCursor cursorCache[ImGuiMouseCursor_COUNT];

	std::chrono::steady_clock::time_point lastFrame;

	bool alwaysSkipKeyboard, alwaysSkipMouseMove, alwaysSkipMouseWheel, alwaysSkipMouseButton;

	void (*Refresh)(void);
};


static ImGui_ImplWX_Data* ImGui_ImplWX_GetBackendData()
{
	return ImGui::GetCurrentContext()? (ImGui_ImplWX_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}
static void ImGui_ImplWX_TriggerRefresh()
{
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	if (bd && bd->Refresh)
		bd->Refresh();
}

//bool ImGui_ImplWX_Init(void (*RefreshCallback)(void))
bool ImGui_ImplWX_Init()
{
	ImGuiIO &io = ImGui::GetIO();

	IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

	ImGui_ImplWX_Data* bd = IM_NEW(ImGui_ImplWX_Data)();
	//bd->Refresh = RefreshCallback;
	bd->lastFrame = std::chrono::steady_clock::now();

	io.BackendPlatformUserData = (void*)bd;
	io.BackendPlatformName = "imgui_impl_wx";

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

	io.MouseDrawCursor = false;

	bd->lastMouseCursor = ImGuiMouseCursor_COUNT;
	bd->cursorCache[ImGuiMouseCursor_Arrow] = wxCursor(wxCURSOR_ARROW);
	bd->cursorCache[ImGuiMouseCursor_TextInput] = wxCursor(wxCURSOR_CHAR);
	bd->cursorCache[ImGuiMouseCursor_ResizeAll] = wxCursor(wxCURSOR_SIZENWSE); // Used for corner resizing, so need NWSE, not SIZING, which is moving
	bd->cursorCache[ImGuiMouseCursor_ResizeNS] = wxCursor(wxCURSOR_SIZENS);
	bd->cursorCache[ImGuiMouseCursor_ResizeEW] = wxCursor(wxCURSOR_SIZEWE);
	bd->cursorCache[ImGuiMouseCursor_ResizeNESW] = wxCursor(wxCURSOR_SIZENESW);
	bd->cursorCache[ImGuiMouseCursor_ResizeNWSE] = wxCursor(wxCURSOR_SIZENWSE);
	bd->cursorCache[ImGuiMouseCursor_Hand] = wxCursor(wxCURSOR_HAND);
	bd->cursorCache[ImGuiMouseCursor_NotAllowed] = wxCursor(wxCURSOR_NO_ENTRY);

	io.SetClipboardTextFn = ImGui_ImplWX_SetClipboardText;
	io.GetClipboardTextFn = ImGui_ImplWX_GetClipboardText;

	bd->alwaysSkipKeyboard = bd->alwaysSkipMouseMove = bd->alwaysSkipMouseWheel = bd->alwaysSkipMouseButton = false;
	bd->alwaysSkipMouseButton = true;

	return true;
}

void ImGui_ImplWX_Shutdown()
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors);
    IM_DELETE(bd);
}

void ImGui_ImplWX_NewFrame(wxWindow *canvas)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();

#ifdef _WIN32
	ImGui::GetMainViewport() ->PlatformHandleRaw = canvas->GetHWND();
#endif

	// Update with canvas size (real estate to render to)
	wxSize cs = canvas->GetClientSize();
	float sf = canvas->GetContentScaleFactor();
	io.DisplaySize = ImVec2(cs.x, cs.y);
	io.DisplayFramebufferScale = ImVec2(sf, sf);

	// Setup time step
	auto now = std::chrono::steady_clock::now();
	float dT = std::chrono::duration_cast<std::chrono::milliseconds>(now - bd->lastFrame).count() / 1000.0f;
	if (dT > 0.001f)
		io.DeltaTime = dT;
	else
		printf("Only had delta time of %f!\n", dT);
	bd->lastFrame = now;

	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
	{
		ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
		if (imguiCursor != bd->lastMouseCursor)
		{
			if (io.MouseDrawCursor || imguiCursor == ImGuiMouseCursor_None)
				wxSetCursor(wxCursor(wxCURSOR_NONE));
			else
				wxSetCursor(bd->cursorCache[imguiCursor]);
			bd->lastMouseCursor = imguiCursor;
		}
	}
}

const char* ImGui_ImplWX_GetClipboardText(void *user_data)
{
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();

	bd->clipboardText = "";
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject clipboardData;
			wxTheClipboard->GetData(clipboardData);
			bd->clipboardText = clipboardData.GetText().c_str();
		}
		wxTheClipboard->Close();
	}

	return bd->clipboardText.c_str();
}

void ImGui_ImplWX_SetClipboardText(void *user_data, const char *text)
{
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(text));
		wxTheClipboard->Close();
	}
}

static ImGuiKey TranslateKeys(int key)
{
	switch (key)
	{
		case WXK_TAB: return ImGuiKey_Tab;
		case WXK_LEFT: return ImGuiKey_LeftArrow;
		case WXK_RIGHT: return ImGuiKey_RightArrow;
		case WXK_UP: return ImGuiKey_UpArrow;
		case WXK_DOWN: return ImGuiKey_DownArrow;
		case WXK_PAGEUP: return ImGuiKey_PageUp;
		case WXK_PAGEDOWN: return ImGuiKey_PageDown;
		case WXK_HOME: return ImGuiKey_Home;
		case WXK_END: return ImGuiKey_End;
		case WXK_INSERT: return ImGuiKey_Insert;
		case WXK_DELETE: return ImGuiKey_Delete;
		case WXK_BACK: return ImGuiKey_Backspace;
		case WXK_SPACE: return ImGuiKey_Space;
		case WXK_RETURN: return ImGuiKey_Enter;
		case WXK_ESCAPE: return ImGuiKey_Escape;
		case WXK_CONTROL: return ImGuiKey_LeftCtrl;
		case WXK_SHIFT: return ImGuiKey_LeftShift;
		case WXK_ALT: return ImGuiKey_LeftAlt;
		case WXK_WINDOWS_LEFT: return ImGuiKey_LeftSuper;
		//case WXK_CONTROL: return ImGuiKey_RightCtrl;
		//case WXK_SHIFT: return ImGuiKey_RightShift;
		//case WXK_ALT: return ImGuiKey_RightAlt;
		case WXK_WINDOWS_RIGHT: return ImGuiKey_RightSuper;
		case WXK_MENU: return ImGuiKey_Menu;

		case WXK_NUMPAD_TAB: return ImGuiKey_Tab;
		case WXK_NUMPAD_LEFT: return ImGuiKey_LeftArrow;
		case WXK_NUMPAD_RIGHT: return ImGuiKey_RightArrow;
		case WXK_NUMPAD_UP: return ImGuiKey_UpArrow;
		case WXK_NUMPAD_DOWN: return ImGuiKey_DownArrow;
		case WXK_NUMPAD_PAGEUP: return ImGuiKey_PageUp;
		case WXK_NUMPAD_PAGEDOWN: return ImGuiKey_PageDown;
		case WXK_NUMPAD_HOME: return ImGuiKey_Home;
		case WXK_NUMPAD_END: return ImGuiKey_End;
		case WXK_NUMPAD_INSERT: return ImGuiKey_Insert;
		case WXK_NUMPAD_DELETE: return ImGuiKey_Delete;
		case WXK_NUMPAD_SPACE: return ImGuiKey_Space;
		case WXK_NUMPAD_F1: return ImGuiKey_F1;
		case WXK_NUMPAD_F2: return ImGuiKey_F2;
		case WXK_NUMPAD_F3: return ImGuiKey_F3;
		case WXK_NUMPAD_F4: return ImGuiKey_F4;

		
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;

		case 'A': case 'a': return ImGuiKey_A;
		case 'B': case 'b': return ImGuiKey_B;
		case 'C': case 'c': return ImGuiKey_C;
		case 'D': case 'd': return ImGuiKey_D;
		case 'E': case 'e': return ImGuiKey_E;
		case 'F': case 'f': return ImGuiKey_F;
		case 'G': case 'g': return ImGuiKey_G;
		case 'H': case 'h': return ImGuiKey_H;
		case 'I': case 'i': return ImGuiKey_I;
		case 'J': case 'j': return ImGuiKey_J;
		case 'K': case 'k': return ImGuiKey_K;
		case 'L': case 'l': return ImGuiKey_L;
		case 'M': case 'm': return ImGuiKey_M;
		case 'N': case 'n': return ImGuiKey_N;
		case 'O': case 'o': return ImGuiKey_O;
		case 'P': case 'p': return ImGuiKey_P;
		case 'Q': case 'q': return ImGuiKey_Q;
		case 'R': case 'r': return ImGuiKey_R;
		case 'S': case 's': return ImGuiKey_S;
		case 'T': case 't': return ImGuiKey_T;
		case 'U': case 'u': return ImGuiKey_U;
		case 'V': case 'v': return ImGuiKey_V;
		case 'W': case 'w': return ImGuiKey_W;
		case 'X': case 'x': return ImGuiKey_X;
		case 'Y': case 'y': return ImGuiKey_Y;
		case 'Z': case 'z': return ImGuiKey_Z;

		case WXK_F1: return ImGuiKey_F1;
		case WXK_F2: return ImGuiKey_F2;
		case WXK_F3: return ImGuiKey_F3;
		case WXK_F4: return ImGuiKey_F4;
		case WXK_F5: return ImGuiKey_F5;
		case WXK_F6: return ImGuiKey_F6;
		case WXK_F7: return ImGuiKey_F7;
		case WXK_F8: return ImGuiKey_F8;
		case WXK_F9: return ImGuiKey_F9;
		case WXK_F10: return ImGuiKey_F10;
		case WXK_F11: return ImGuiKey_F11;
		case WXK_F12: return ImGuiKey_F12;

		case '"': return ImGuiKey_Apostrophe;
		case ',': return ImGuiKey_Comma;
		case '-': return ImGuiKey_Minus;
		case '.': return ImGuiKey_Period;
		case '/': return ImGuiKey_Slash;
		case ';': return ImGuiKey_Semicolon;
		case '=': return ImGuiKey_Equal;
		case '(': return ImGuiKey_LeftBracket;
		case '\\': return ImGuiKey_Backslash;
		case ')': return ImGuiKey_RightBracket;
		case '`': return ImGuiKey_GraveAccent;
		case WXK_CAPITAL: return ImGuiKey_CapsLock;
		case WXK_SCROLL: return ImGuiKey_ScrollLock;
		case WXK_NUMLOCK: return ImGuiKey_NumLock;
		case WXK_PRINT: return ImGuiKey_PrintScreen;
		case WXK_PAUSE: return ImGuiKey_Pause;
		
		case WXK_NUMPAD0: return ImGuiKey_Keypad0;
		case WXK_NUMPAD1: return ImGuiKey_Keypad1;
		case WXK_NUMPAD2: return ImGuiKey_Keypad2;
		case WXK_NUMPAD3: return ImGuiKey_Keypad3;
		case WXK_NUMPAD4: return ImGuiKey_Keypad4;
		case WXK_NUMPAD5: return ImGuiKey_Keypad5;
		case WXK_NUMPAD6: return ImGuiKey_Keypad6;
		case WXK_NUMPAD7: return ImGuiKey_Keypad7;
		case WXK_NUMPAD8: return ImGuiKey_Keypad8;
		case WXK_NUMPAD9: return ImGuiKey_Keypad9;
		case WXK_NUMPAD_DECIMAL: return ImGuiKey_KeypadDecimal;
		case WXK_NUMPAD_DIVIDE: return ImGuiKey_KeypadDivide;
		case WXK_NUMPAD_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case WXK_NUMPAD_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case WXK_NUMPAD_ADD: return ImGuiKey_KeypadAdd;
		case WXK_NUMPAD_ENTER: return ImGuiKey_KeypadEnter;
		case WXK_NUMPAD_EQUAL: return ImGuiKey_KeypadEqual;
		default:
			printf("Unknown char %c or keycode %d\n", (char)key, key);
			return ImGuiKey_None;
	}
}

void ImGui_ImplWX_OnKeyDown(wxKeyEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();

	ImGuiKey key = TranslateKeys(event.GetKeyCode());
	io.AddKeyEvent(key, true);

	int modifiers = event.GetModifiers();
	io.AddKeyEvent(ImGuiMod_Ctrl, modifiers&wxMOD_CONTROL);
	io.AddKeyEvent(ImGuiMod_Shift, modifiers&wxMOD_SHIFT);
	io.AddKeyEvent(ImGuiMod_Alt, modifiers&wxMOD_ALT);
	io.AddKeyEvent(ImGuiMod_Super, modifiers&wxMOD_META);

	if (!io.WantCaptureKeyboard || bd->alwaysSkipKeyboard)
		event.Skip();
	//if (io.WantCaptureKeyboard)
		ImGui_ImplWX_TriggerRefresh();
}

void ImGui_ImplWX_OnKeyUp(wxKeyEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();

	io.AddKeyEvent(TranslateKeys(event.GetKeyCode()), false);

	int modifiers = event.GetModifiers();
	io.AddKeyEvent(ImGuiMod_Ctrl, modifiers&wxMOD_CONTROL);
	io.AddKeyEvent(ImGuiMod_Shift, modifiers&wxMOD_SHIFT);
	io.AddKeyEvent(ImGuiMod_Alt, modifiers&wxMOD_ALT);
	io.AddKeyEvent(ImGuiMod_Super, modifiers&wxMOD_META);

	if (!io.WantCaptureKeyboard || bd->alwaysSkipKeyboard)
		event.Skip();
	//if (io.WantCaptureKeyboard)
		ImGui_ImplWX_TriggerRefresh();
}

void ImGui_ImplWX_OnChar(wxKeyEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	wxChar uc = event.GetUnicodeKey();
	if (uc != WXK_NONE)
	{
		io.AddInputCharacter(uc);
	}
	else // No Unicode equivalent.
	{
		unsigned int c = event.GetRawKeyCode();
		if (c > 0 && c < 1000)
			io.AddInputCharacter(c);
	}

	event.DoAllowNextEvent();

	//if (!io.WantCaptureKeyboard)
		event.Skip();
	//else
		ImGui_ImplWX_TriggerRefresh();
}
void ImGui_ImplWX_OnMouseMoveEvent(wxMouseEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	io.AddMousePosEvent(event.GetX(), event.GetY());
	if (!io.WantCaptureMouse || bd->alwaysSkipMouseMove)
		event.Skip();
	//if (io.WantCaptureMouse)
		ImGui_ImplWX_TriggerRefresh();
}
void ImGui_ImplWX_OnMouseWheelEvent(wxMouseEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
		io.AddMouseWheelEvent(0, event.GetWheelRotation());
	else if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
		io.AddMouseWheelEvent(event.GetWheelRotation(), 0);
	if (!io.WantCaptureMouse || bd->alwaysSkipMouseWheel)
		event.Skip();
	//if (io.WantCaptureMouse)
		ImGui_ImplWX_TriggerRefresh();
}

void ImGui_ImplWX_OnMouseLeftEvent(wxMouseEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	io.AddMouseButtonEvent(0, event.LeftIsDown());
	if (!io.WantCaptureMouse || bd->alwaysSkipMouseButton)
		event.Skip();
	if (io.WantCaptureMouse)
		ImGui_ImplWX_TriggerRefresh();
}
void ImGui_ImplWX_OnMouseRightEvent(wxMouseEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	io.AddMouseButtonEvent(1, event.RightIsDown());
	if (!io.WantCaptureMouse || bd->alwaysSkipMouseButton)
		event.Skip();
	//if (io.WantCaptureMouse)
		ImGui_ImplWX_TriggerRefresh();
}
void ImGui_ImplWX_OnMouseMiddleEvent(wxMouseEvent &event)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplWX_Data *bd = ImGui_ImplWX_GetBackendData();
	io.AddMouseButtonEvent(20, event.MiddleIsDown());
	if (!io.WantCaptureMouse || bd->alwaysSkipMouseButton)
		event.Skip();
	//if (io.WantCaptureMouse)
		ImGui_ImplWX_TriggerRefresh();
}
