
#include "StdAfx.h"
#include "Application.h"
#include "Input.h"

namespace MicroSDK
{

Application*	g_pApplicationInstance	= NULL;
HWND			g_hApplicationWindow	= 0;

Application::Application(ApplicationType a_eApplicationType)
{
	m_eApplicationType = a_eApplicationType;
	g_pApplicationInstance = this;	
}

Application::~Application()
{
}


void Application::OnInitialize()
{
	// default impl: empty
}

void Application::OnShutdown()
{
	// default impl: empty
}

void Application::OnUpdate()
{
	// default impl: empty
}

void Application::OnRender()
{
	// default impl: empty
}

void Application::Run()
{
	OnInitialize();

	//
	MSG msg;

	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				break;
		}
		else
		{
			OnUpdate();
			OnRender();			
		}
	}

	OnShutdown();
}



Input::Key MapKey( UINT nWinKey )
{
    // This could be upgraded to a method that's user-definable but for 
    // simplicity, we'll use a hardcoded mapping.
    switch( nWinKey )
    {
        case VK_CONTROL:
            return Input::Key_Control;
        case VK_LEFT:
            return Input::Key_Left;
        case VK_RIGHT:
            return Input::Key_Right;
        case VK_UP:
            return Input::Key_Up;
        case VK_DOWN:
            return Input::Key_Down;

		case 13:
			return Input::Key_Enter;
		case ' ':
			return Input::Key_Space;
        case 'A':
            return Input::Key_A;
        case 'B':
            return Input::Key_B;
        case 'C':
            return Input::Key_C;
        case 'D':
            return Input::Key_D;
        case 'E':
            return Input::Key_E;
        case 'F':
            return Input::Key_F;
        case 'G':
            return Input::Key_G;
        case 'H':
            return Input::Key_H;
        case 'I':
            return Input::Key_I;
        case 'J':
            return Input::Key_J;
        case 'K':
            return Input::Key_K;
        case 'L':
            return Input::Key_L;
        case 'M':
            return Input::Key_M;
        case 'N':
            return Input::Key_N;
        case 'O':
            return Input::Key_O;
        case 'P':
            return Input::Key_P;
        case 'Q':
            return Input::Key_Q;
        case 'R':
            return Input::Key_R;
        case 'S':
            return Input::Key_S;
        case 'T':
            return Input::Key_T;
        case 'U':
            return Input::Key_U;
        case 'V':
            return Input::Key_V;
        case 'W':
            return Input::Key_W;
        case 'X':
            return Input::Key_X;
        case 'Y':
            return Input::Key_Y;
        case 'Z':
            return Input::Key_Z;
        case VK_HOME:
            return Input::Key_Home;
    }

    return Input::Key_Invalid;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
			break;
		}

        case WM_KEYDOWN:
        {
			Input::Key eMappedKey = MapKey((UINT)wParam );

            if (eMappedKey != Input::Key_Invalid)
            {
				Input::HandleKeyDownMessage(eMappedKey);
            }
            break;
        }

        case WM_KEYUP:
        {
            Input::Key eMappedKey = MapKey((UINT)wParam );

            if (eMappedKey != Input::Key_Invalid)
            {
				Input::HandleKeyUpMessage(eMappedKey);
            }
            break;
        }
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}


} // namespace MicroSDK



int main(int argc, char** argv)
{
	MicroSDK::g_hApplicationWindow = 0;
	MicroSDK::g_pApplicationInstance->Run();
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (MicroSDK::g_pApplicationInstance->GetApplicationType() == MicroSDK::ApplicationType_Window)
	{
		HWND hWnd;
		WNDCLASSEX wc;

		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = MicroSDK::WindowProc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = L"WindowClass";

		RegisterClassEx(&wc);

		RECT wr = {0, 0, 800, 600};
		RECT adjustedWr = wr;
		AdjustWindowRect(&adjustedWr, WS_OVERLAPPEDWINDOW, FALSE);

		hWnd = CreateWindowEx(NULL,
			L"WindowClass",
			L"Direct3D",
			WS_OVERLAPPEDWINDOW,
			300,
			300,
			adjustedWr.right - adjustedWr.left,
			adjustedWr.bottom - adjustedWr.top,
			NULL,
			NULL,
			hInstance,
			NULL);

		ShowWindow(hWnd, nCmdShow);

		MicroSDK::g_hApplicationWindow = hWnd;
	}

	MicroSDK::g_pApplicationInstance->Run();
}
