#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

void CreateJpgWithText(const char *text, const char *outputFileName) {
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create a bitmap and a graphics object
    Gdiplus::Bitmap bitmap(400, 200, PixelFormat24bppRGB);
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

    // Save the bitmap as a JPG file
    CLSID clsidJpg;
    GetEncoderClsid(L"image/jpeg", &clsidJpg);
    bitmap.Save(outputFileName, &clsidJpg, NULL);

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

int main() {
    const char *text = "Hello, JPG!";
    const char *outputFileName = "output.jpg";

    CreateJpgWithText(text, outputFileName);

    return 0;
}
