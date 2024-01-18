#include <windows.h>
#include <wingdi.h>

void CreateGifWithText(const char *text, const char *outputFileName) {
    // Create a memory DC
    HDC hdc = CreateCompatibleDC(NULL);

    // Set the desired image dimensions
    int width = 400;
    int height = 200;

    // Create a bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);

    // Select the bitmap into the memory DC
    SelectObject(hdc, hBitmap);

    // Set the background color
    SetBkColor(hdc, RGB(255, 255, 255));

    // Set the text color
    SetTextColor(hdc, RGB(0, 0, 0));

    // Draw the text on the bitmap
    TextOutA(hdc, 10, 50, text, strlen(text));

    // Save the bitmap as a GIF file
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID clsidEncoder;
    GetEncoderClsid(L"image/gif", &clsidEncoder);

    Bitmap *bitmap = new Bitmap(hBitmap, NULL);
    bitmap->Save(outputFileName, &clsidEncoder, NULL);

    GdiplusShutdown(gdiplusToken);

    // Clean up resources
    DeleteObject(hBitmap);
    DeleteDC(hdc);
}

int main() {
    const char *text = "Hello, GIF!";
    const char *outputFileName = "output.gif";

    CreateGifWithText(text, outputFileName);

    return 0;
}
