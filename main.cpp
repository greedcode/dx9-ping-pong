#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int PADDLE_SPEED = 5;

const int BALL_SIZE = 20;
const int BALL_SPEED_X = 6.5;
const int BALL_SPEED_Y = 6.5;

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3d_device;
LPDIRECT3DVERTEXBUFFER9 vertex_buffer_paddle1;
LPDIRECT3DVERTEXBUFFER9 vertex_buffer_paddle2;
LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ball;

int paddle1_y;
int paddle2_y;

int ball_x;
int ball_y;
int ball_speed_x;
int ball_speed_y;

bool is_paused = false;

ID3DXFont* font = NULL;
const int FONT_SIZE = 36;

struct CUSTOMVERTEX
{
    float x, y, z, rhw;
    DWORD color;
};

const int TEXT_WIDTH = 200;
const int TEXT_HEIGHT = 100;

int paddle1_score = 0;
int paddle2_score = 0;

bool init_d3d(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) return false;

    D3DPRESENT_PARAMETERS d3dpp = { 0 };
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;

    if (FAILED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &d3dpp,
        &d3d_device))) return false;

    d3d_device->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vertex_buffer_paddle1, NULL);
    d3d_device->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vertex_buffer_paddle2, NULL);
    d3d_device->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &vertex_buffer_ball, NULL);

    return true;
}

void init_game()
{
    paddle1_y = paddle2_y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2;
    ball_x = ball_y = (WINDOW_WIDTH - BALL_SIZE) / 2;
    ball_speed_x = BALL_SPEED_X;
    ball_speed_y = BALL_SPEED_Y;
}

void update_game()
{
    if (GetAsyncKeyState(VK_UP) & 0x8000 && paddle2_y > 0)
        paddle2_y -= PADDLE_SPEED;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000 && paddle2_y < WINDOW_HEIGHT - PADDLE_HEIGHT)
        paddle2_y += PADDLE_SPEED;
    if (GetAsyncKeyState('W') & 0x8000 && paddle1_y > 0)
        paddle1_y -= PADDLE_SPEED;
    if (GetAsyncKeyState('S') & 0x8000 && paddle1_y < WINDOW_HEIGHT - PADDLE_HEIGHT)
        paddle1_y += PADDLE_SPEED;

    ball_x += ball_speed_x;
    ball_y += ball_speed_y;

    if (ball_x < PADDLE_WIDTH && ball_y + BALL_SIZE >= paddle1_y && ball_y <= paddle1_y + PADDLE_HEIGHT)
    {
        if (ball_speed_x < 0)
        {
            paddle1_score++;
            ball_speed_x = -ball_speed_x;
        }
        else
        {
            ball_speed_y = -ball_speed_y;
        }
    }
    else if (ball_x + BALL_SIZE > WINDOW_WIDTH - PADDLE_WIDTH * 2 && ball_y + BALL_SIZE >= paddle2_y && ball_y <= paddle2_y + PADDLE_HEIGHT)
    {
        if (ball_speed_x > 0)
        {
            paddle2_score++;
            ball_speed_x = -ball_speed_x;
        }
        else
        {
            ball_speed_y = -ball_speed_y;
        }
    }

    if (ball_x <= 0)
    {
        ball_speed_x = -ball_speed_x;
        paddle1_score = 0;
    }
    else if (ball_x + BALL_SIZE >= WINDOW_WIDTH)
    {
        ball_speed_x = -ball_speed_x;
        paddle2_score = 0;
    }

    if (ball_y <= 0 || ball_y + BALL_SIZE >= WINDOW_HEIGHT)
        ball_speed_y = -ball_speed_y;

    if (ball_x < 0 || ball_x > WINDOW_WIDTH - BALL_SIZE)
    {
        ball_x = (WINDOW_WIDTH - BALL_SIZE) / 2;
        ball_y = (WINDOW_HEIGHT - BALL_SIZE) / 2;
        ball_speed_x = BALL_SPEED_X;
        ball_speed_y = BALL_SPEED_Y;
    }
}

void render_game()
{
    d3d_device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    if (SUCCEEDED(d3d_device->BeginScene()))
    {
        if (is_paused)
        {
            RECT rect = { 0, WINDOW_HEIGHT / 4, WINDOW_WIDTH, WINDOW_HEIGHT / 4 + FONT_SIZE };
            font->DrawText(NULL, TEXT("Paused"), -1, &rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));
        }

        CUSTOMVERTEX vertices_paddle1[] =
        {
            { 0, paddle1_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { PADDLE_WIDTH, paddle1_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { 0, paddle1_y + PADDLE_HEIGHT, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { PADDLE_WIDTH, paddle1_y + PADDLE_HEIGHT, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) }
        };
        void* p_vertices_paddle1;
        vertex_buffer_paddle1->Lock(0, sizeof(vertices_paddle1), &p_vertices_paddle1, 0);
        memcpy(p_vertices_paddle1, vertices_paddle1, sizeof(vertices_paddle1));
        vertex_buffer_paddle1->Unlock();

        d3d_device->SetStreamSource(0, vertex_buffer_paddle1, 0, sizeof(CUSTOMVERTEX));
        d3d_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        d3d_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        CUSTOMVERTEX vertices_paddle2[] =
        {
            { WINDOW_WIDTH - PADDLE_WIDTH * 2, paddle2_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { WINDOW_WIDTH, paddle2_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { WINDOW_WIDTH - PADDLE_WIDTH * 2, paddle2_y + PADDLE_HEIGHT, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { WINDOW_WIDTH, paddle2_y + PADDLE_HEIGHT, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) }
        };
        void* p_vertices_paddle2;
        vertex_buffer_paddle2->Lock(0, sizeof(vertices_paddle2), &p_vertices_paddle2, 0);
        memcpy(p_vertices_paddle2, vertices_paddle2, sizeof(vertices_paddle2));
        vertex_buffer_paddle2->Unlock();

        d3d_device->SetStreamSource(0, vertex_buffer_paddle2, 0, sizeof(CUSTOMVERTEX));
        d3d_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        d3d_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        CUSTOMVERTEX vertices_ball[] =
        {
            { ball_x, ball_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { ball_x + BALL_SIZE, ball_y, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { ball_x, ball_y + BALL_SIZE, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) },
            { ball_x + BALL_SIZE, ball_y + BALL_SIZE, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) }
        };
        void* p_vertices_ball;
        vertex_buffer_ball->Lock(0, sizeof(vertices_ball), &p_vertices_ball, 0);
        memcpy(p_vertices_ball, vertices_ball, sizeof(vertices_ball));
        vertex_buffer_ball->Unlock();

        d3d_device->SetStreamSource(0, vertex_buffer_ball, 0, sizeof(CUSTOMVERTEX));
        d3d_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        d3d_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        TCHAR score_text[32];
        _stprintf_s(score_text, TEXT("%d"), paddle1_score);
        RECT paddle1_score_rect = { WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8, WINDOW_WIDTH / 4 + FONT_SIZE, WINDOW_HEIGHT / 8 + FONT_SIZE };
        font->DrawText(NULL, score_text, -1, &paddle1_score_rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));

        _stprintf_s(score_text, TEXT("%d"), paddle2_score);
        RECT paddle2_score_rect = { 3 * WINDOW_WIDTH / 4, WINDOW_HEIGHT / 8, 3 * WINDOW_WIDTH / 4 + FONT_SIZE, WINDOW_HEIGHT / 8 + FONT_SIZE };
        font->DrawText(NULL, score_text, -1, &paddle2_score_rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));

        RECT player1_label_rect = { WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, WINDOW_WIDTH / 8 + FONT_SIZE, WINDOW_HEIGHT / 8 + FONT_SIZE };
        font->DrawText(NULL, _T("P1"), -1, &player1_label_rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));

        RECT player2_label_rect = { WINDOW_WIDTH - WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, WINDOW_WIDTH - WINDOW_WIDTH / 8 + FONT_SIZE, WINDOW_HEIGHT / 8 + FONT_SIZE };
        font->DrawText(NULL, _T("P2"), -1, &player2_label_rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));

        if (paddle1_score >= 10 || paddle2_score >= 10)
        {
            RECT winner_rect = { WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2 + 50 };
            RECT reset_rect = { WINDOW_WIDTH / 2 - 150, winner_rect.bottom + 10, WINDOW_WIDTH / 2 + 160, winner_rect.bottom + 60 };
            TCHAR winner_text[32];

            _stprintf_s(winner_text, TEXT("P%d Won"), (paddle1_score >= 10) ? 1 : 2);

            font->DrawText(NULL, winner_text, -1, &winner_rect, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(255, 255, 255));
            font->DrawText(NULL, _T("Press Enter to restart"), -1, &reset_rect, DT_CENTER | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));

            is_paused = true;
        }

        d3d_device->EndScene();
        d3d_device->Present(NULL, NULL, NULL, NULL);
    }
}

void cleanup()
{
    vertex_buffer_paddle1 && vertex_buffer_paddle1->Release(), vertex_buffer_paddle1 = NULL;
    vertex_buffer_paddle2 && vertex_buffer_paddle2->Release(), vertex_buffer_paddle2 = NULL;
    vertex_buffer_ball && vertex_buffer_ball->Release(), vertex_buffer_ball = NULL;
    font && font->Release(), font = NULL;
    d3d_device && d3d_device->Release(), d3d_device = NULL;
    d3d && d3d->Release(), d3d = NULL;
}

bool create_font()
{
    return D3DXCreateFont(d3d_device, TEXT_HEIGHT, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font) != NULL;
}

LRESULT CALLBACK window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_SPACE) is_paused = !is_paused;
        if (is_paused && (paddle1_score == 10 || paddle2_score == 10) && wParam == VK_RETURN) {
            paddle1_score = 0;
            paddle2_score = 0;
            ball_x = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
            ball_y = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
            ball_speed_x = BALL_SPEED_X;
            ball_speed_y = BALL_SPEED_Y;
            is_paused = false;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, window_proc, NULL, NULL, hInstance, NULL, LoadCursor(NULL, IDC_ARROW), (HBRUSH)COLOR_WINDOW, NULL, TEXT("PingPongGame"), NULL};
    if (!RegisterClassEx(&wc)) return -1;
    HWND hWnd = CreateWindowEx(0, TEXT("PingPongGame"), TEXT("Ping Pong Game"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    if (!hWnd) return -1;
    if (!init_d3d(hWnd)) return -1;
    D3DXCreateFont(d3d_device, FONT_SIZE, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &font);
    init_game();
    ShowWindow(hWnd, nCmdShow);
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!is_paused) update_game();
        render_game();
    }
    cleanup();
    UnregisterClass(TEXT("PingPongGame"), wc.hInstance);
    return 0;
}