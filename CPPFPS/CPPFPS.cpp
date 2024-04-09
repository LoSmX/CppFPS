// CPPFPS.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <chrono>
#include <vector>

#include <Windows.h>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;
int nScreenSize = nScreenHeight * nScreenWidth;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;  // anlge player looking

int nMapWidth = 16;
int nMapHeight = 16;
int nMapSize = nMapWidth * nMapHeight;
float fFOV = 3.14f / 4.0f;
float fHalfFOV = fFOV / 2.0f;
float fDepth = 16;

int main()
{
    // Create Screen buffer
    wchar_t* screen = new wchar_t[nScreenSize];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Set the size of the console screen buffer
    COORD bufferSize = { nScreenWidth, nScreenHeight}; // Set width and height as desired
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize)) {
        std::cerr << "Failed to set console screen buffer size." << std::endl;
        CloseHandle(hConsole);
        return 1;
    }

    // Set the size and position of the console window
    SMALL_RECT windowSize = { 0, 0, nScreenWidth - 1, nScreenHeight - 1 }; // Adjust as needed
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
        std::cerr << "Failed to set console window size." << std::endl;
        CloseHandle(hConsole);
        return 1;
    }

    std::wstring map;

    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#....######....#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..###....######";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game Loop
    while (1) {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Controls
        // HAndle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (1.0f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (1.0f) * fElapsedTime;

        // Move forward with w
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000){
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            // Check if coliding with wall
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        // Move backwards with s
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            // Check if coliding with wall
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        // For each collum on screen
        for (int x = 0; x < nScreenWidth; x++) {
            // Angle from begining to and of screem
            float fRayAngle = (fPlayerA - fHalfFOV) + ((float)x / (float)nScreenWidth) * fFOV;

            // Calculating distance to wall for each column.
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;

                // 
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Test if Ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;            // Just set Distance to max depth
                    fDistanceToWall = fDepth;
                }
                else {
                    // Ray is inbounds check if ray hits the wall
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        // Check where are the corneers of wall
                        vector<pair<float, float>> p; // distance, dot

                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestY + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }
                    }
                }
            }
            // Calculate distance to ceiling and floor
            int nCeiling = (int)( (float)(nScreenHeight / 2.0f - nScreenHeight / ((float)fDistanceToWall)));
            int nFloor = nScreenHeight - nCeiling;

            // Shader map
            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f)           nShade = 0x2588;    // Very Close
            else if (fDistanceToWall <= fDepth / 3.0f)      nShade = 0x2593;
            else if (fDistanceToWall <= fDepth / 2.0f)      nShade = 0x2592;
            else if (fDistanceToWall <= fDepth)             nShade = 0x2591;
            else                                            nShade = ' ';       // Too far away

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling) {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if(y >= nCeiling && y <= nFloor)
                {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else
                {
                    // Shader map
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25f)              nShade = '#';    // Very Close
                    else if (b < 0.50f)         nShade = 'x';
                    else if (b < 0.75f)         nShade = '-';
                    else                        nShade = '.';
                    screen[y * nScreenWidth + x] = nShade;
                }
            }
        }

        // screen[nScreenSize - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenSize, { 0,0 }, &dwBytesWritten);
    }

    // Close the console screen buffer handle when done
    CloseHandle(hConsole);

    return 0;
}
