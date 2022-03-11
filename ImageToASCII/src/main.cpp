#include <iostream>
#include <fstream>
#include <SFML/Graphics/Image.hpp>
#include <format>

std::wstring get_char(uint8_t l, bool mode)
{
    if (l >= 210)
        return mode == 1 ? L"██" : L"  ";
    else if (l >= 150 && l < 210)
        return mode == 1 ? L"▓▓" : L"░░";
    else if (l >= 90 && l < 150)
        return L"▒▒";
    else if (l >= 50 && l < 90)
        return mode == 1 ? L"░░" : L"▓▓";
    else if (l < 50)
        return mode == 1 ? L"  " : L"██";
    return {};
}

double inv_gam_sRGB(int ic)
{
    double c{ ic / 255.0 };
    if (c <= 0.04045)
        return c / 12.92;
    else
        return pow(((c + 0.055) / (1.055)), 2.4);
}

uint8_t gam_sRGB(double v)
{
    if (v <= 0.0031308)
        v *= 12.92;
    else
        v = 1.055 * pow(v, 1.0 / 2.4) - 0.055;
    return int(v * 255 + 0.5);
}

uint8_t get_luminance(uint8_t r, uint8_t g, uint8_t b)
{
    return gam_sRGB(0.212655 * inv_gam_sRGB(r) + 0.715158 * inv_gam_sRGB(g) + 0.072187 * inv_gam_sRGB(b));
}

size_t last_index(std::string& str, char x)
{
    for (size_t i{ str.length() }; i-- > 0;)
    {
        if (str[i] == x)
            return i;
    }
    return 0;
}

void asciify(sf::Image& image, std::string& path, bool mode)
{
    std::wofstream textFile(path.substr(0, last_index(path, '.')) + ".txt");
    for (uint32_t y{ 0 }; y < image.getSize().y; y++)
    {
        for (uint32_t x{ 0 }; x < image.getSize().x; x++)
        {
            sf::Color color = image.getPixel(x, y);
            if (color.a < 20)
                textFile << (mode == 0 ? L"  " : L"██");
            else
                textFile << get_char(get_luminance(color.r, color.g, color.b), mode);
        }
        textFile << std::endl;
    }
}

sf::Image resize_image(sf::Image& originalImage, uint32_t targetWidth)
{
    sf::Vector2u size{ originalImage.getSize() };
    sf::Vector2u newSize;

    uint32_t max_dim{ size.x >= size.y ? size.x : size.y };
    float scale{ static_cast<float>(targetWidth) / max_dim };

    if (size.x >= size.y)
    {
        newSize.x = targetWidth;
        newSize.y = static_cast<uint32_t>(size.y * scale);
    }
    else
    {
        newSize.y = targetWidth;
        newSize.x = static_cast<uint32_t>(size.x * scale);
    }

    sf::Image resizedImage;
    resizedImage.create(newSize.x, newSize.y);

    for (uint32_t y{ 0 }; y < newSize.y; y++)
    {
        for (uint32_t x{ 0 }; x < newSize.x; x++)
        {
            uint32_t origX{ static_cast<uint32_t> (static_cast<double>(x) / newSize.x * originalImage.getSize().x) };
            uint32_t origY{ static_cast<uint32_t> (static_cast<double>(y) / newSize.y * originalImage.getSize().y) };
            resizedImage.setPixel(x, y, originalImage.getPixel(origX, origY));
        }
    }
    return resizedImage;
}

bool load_image(std::string& path)
{
    sf::Image image;
    if (image.loadFromFile(path))
    {
        uint32_t width;
        std::cout << std::format("Image size: {0} {1}\n", image.getSize().x, image.getSize().y);
        do {
            std::cout << "Enter target size (type 0 for original): ";
            std::cin >> width;
        } while (width > image.getSize().x);

        if (width != 0)
            image = resize_image(image, width);

        bool mode;
        std::cout << "Black(0), White(1): ";
        std::cin >> mode;
        asciify(image, path, mode);
        return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    std::locale::global(std::locale(""));
    std::string path;

    if (argc == 2)
    {
        path = argv[1];
        if (load_image(path))
            return 0;
    }
    else
    {
        do {
            std::cout << "Type file path: ";
            std::cin >> path;
        } while (!load_image(path));
    }
    return 0;
}