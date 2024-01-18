#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

void CreatePngWithText(const char *text, const char *outputFileName) {
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create a bitmap and a graphics object
    Gdiplus::Bitmap bitmap(400, 200, PixelFormat32bppARGB);
    Gdiplus::Graphics graphics(&bitmap);

    // Set background color
    graphics.Clear(Gdiplus::Color(255, 255, 255));

    // Set the text color and font
    Gdiplus::SolidBrush brush(Gdiplus::Color(0, 0, 0));
    Gdiplus::Font font(L"Arial", 16);
    
    // Draw the text on the bitmap
    graphics.DrawString(
        Gdiplus::WCHAR(text),
        -1,
        &font,
        Gdiplus::PointF(10, 50),
        &brush
    );

    // Save the bitmap as a PNG file
    CLSID clsidPng;
    GetEncoderClsid(L"image/png", &clsidPng);
    bitmap.Save(outputFileName, &clsidPng, NULL);

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

int main() {
    const char *text = "Hello, PNG!";
    const char *outputFileName = "output.png";

    CreatePngWithText(text, outputFileName);

    return 0;
}
