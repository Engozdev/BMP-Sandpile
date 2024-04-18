#include <iostream>
#include <fstream>

#pragma pack(2)

#ifndef LABWORK3_ENGOZDEV_SANDPILE_H
#define LABWORK3_ENGOZDEV_SANDPILE_H

const short kAmountOfDataPelLine = 3, kEnlargeCoefficient = 2, kMaxGrainsPerCell = 4;
const short kMaxDigitAmount = 60, kBitsAlignment = 8;
const uint8_t kFileHeaderSize = 14, kFileHeaderInfo = 40;
const uint8_t kTotalColors = 5, kBitsPerPixel = 4, kBytesPerColor = 4, kPaletteSize = kBytesPerColor * kTotalColors;

struct Args {
    const char* input;
    const char* output;
    int64_t iter_amount, freq;

    Args() {
        input = output = {};
        iter_amount = freq = 0;
    }
};

struct Coords {
    int64_t x, y;
    uint64_t val;
};

struct Node {
    Node* next;
    Coords value;
};

class Queue {
    Node* head;
    Node* tail;
    int64_t size;
public:
    Queue() : head(nullptr), tail(nullptr), size(0) {};

    ~Queue();

    void Push(Coords x);

    Coords Pop();

    bool Empty();
};

Queue::~Queue() {
    Node* temp = tail;
    while (temp != nullptr) {
        temp = tail->next;
        delete tail;
        tail = temp;
    }
}

void Queue::Push(Coords x) {
    size++;
    Node* temp = new Node;
    temp->next = nullptr;
    temp->value = x;

    if (tail == nullptr) {
        head = tail = temp;
    } else {
        head->next = temp;
        head = temp;
    }
}

bool Queue::Empty() {
    return tail == NULL;
}

Coords Queue::Pop() {
    if (tail != nullptr) {
        size--;
        Node* temp = tail;
        Coords res = tail->value;
        tail = tail->next;
        delete temp;
        return res;
    }
}

class SandPile {
    uint64_t** beach;
    int64_t left_border, down_border, right_border, up_border;
    int64_t capacity_n, capacity_m, row_shift, col_shift;
    Queue topple;
public:

    void Initialize(Args arguments);

    bool Topple();

    void EnlargeBeach();

    void SaveBmp(const char* path, uint64_t iterations);
};

void merge(Coords a[], int64_t l, int64_t m, int64_t r) {
    int64_t pos = 0, i = l, j = m + 1;
    Coords temp[r - l + 1];
    while (i != m + 1 && j != r) {
        if (a[i].val <= a[j].val)
            temp[pos++] = a[i++];
        else
            temp[pos++] = a[j++];
    }
    while (i <= m) temp[pos++] = a[i++];
    while (j < r + 1) temp[pos++] = a[j++];

    for (int64_t k = l; k <= r; ++k)
        a[k] = temp[k - l];
}

void mergeSort(Coords a[], int64_t left, int64_t right) {
    if (left == right) return;

    int64_t mid = (left + right) >> 1;
    mergeSort(a, left, mid);
    mergeSort(a, mid + 1, right);
    merge(a, left, mid, right);
}

void SandPile::Initialize(Args arguments) {
    std::ifstream in(arguments.input, std::ios::in | std::ios::binary);
    char ch;
    int64_t cnt = 0, val = 0;
    Coords temp;
    Queue q;
    int64_t min_row = INT16_MAX, min_col = INT16_MAX, max_row = INT16_MIN, max_col = INT16_MIN;
    while (in.get(ch)) {
        if (ch != '\t' && ch != '\r' && ch != '\n')
            val = val * 10 + (ch - '0');
        else if (ch != '\n') {
            if (cnt % kAmountOfDataPelLine == 0) {
                temp.x = static_cast<int64_t>(val);
                min_row = std::min(min_row, temp.x);
                max_row = std::max(max_row, temp.x);
            } else if (cnt % kAmountOfDataPelLine == 1) {
                temp.y = static_cast<int64_t>(val);
                min_col = std::min(min_col, temp.y);
                max_col = std::max(max_col, temp.y);
            } else {
                temp.val = val;
                q.Push(temp);
            }
            cnt = (cnt + 1) % kAmountOfDataPelLine;
            val = 0;
        }
    }
    temp.val = val;
    q.Push(temp);

    capacity_n = max_row - min_row + 1, capacity_m = max_col - min_col + 1;
    left_border = down_border = 0;
    row_shift = col_shift = 0;
    right_border = capacity_m;
    up_border = capacity_n;
    beach = new uint64_t* [capacity_n];
    for (int64_t k = 0; k < capacity_n; ++k) beach[k] = new uint64_t[capacity_m];

    for (int64_t i = 0; i < capacity_n; ++i) {
        for (int64_t j = 0; j < capacity_m; ++j) {
            beach[i][j] = static_cast<uint64_t>(0);
        }
    }
    int64_t row_delta = min_row < 0 ? abs(min_row) : -min_row;
    int64_t col_delta = min_col < 0 ? abs(min_col) : -min_col;
    while (!q.Empty()) {
        temp = q.Pop();
        beach[temp.x + row_delta][temp.y + col_delta] = temp.val;
        if (temp.val >= 4) {
            int64_t x = temp.x + row_delta, y = temp.y + col_delta;
            topple.Push({x, y, temp.val});
        }
    }
}

bool SandPile::Topple() {
    Queue new_topple;
    while (!topple.Empty()) {
        Coords temp = topple.Pop();
        if (beach[row_shift + temp.x][col_shift + temp.y] < kMaxGrainsPerCell) continue;

        beach[row_shift + temp.x][col_shift + temp.y] -= kMaxGrainsPerCell;

        if (temp.x + row_shift == 0 || temp.y + col_shift == 0 || temp.x == capacity_n - 1 ||
            temp.y == capacity_m - 1) {
            EnlargeBeach();
            if (capacity_n < kMaxGrainsPerCell || capacity_m < kMaxGrainsPerCell)
                EnlargeBeach();
        }

        beach[row_shift + temp.x - 1][col_shift + temp.y] += 1;
        if (beach[row_shift + temp.x - 1][col_shift + temp.y] >= kMaxGrainsPerCell) {
            new_topple.Push({static_cast<int64_t>(temp.x - 1), temp.y});
        }

        beach[row_shift + temp.x][col_shift + temp.y - 1] += 1;
        if (beach[row_shift + temp.x][col_shift + temp.y - 1] >= kMaxGrainsPerCell) {
            new_topple.Push({temp.x, static_cast<int64_t>(temp.y - 1)});
        }

        beach[row_shift + temp.x][col_shift + temp.y + 1] += 1;
        if (beach[row_shift + temp.x][col_shift + temp.y + 1] >= kMaxGrainsPerCell) {
            new_topple.Push({temp.x, static_cast<int64_t>(temp.y + 1)});
        }

        beach[row_shift + temp.x + 1][col_shift + temp.y] += 1;
        if (beach[row_shift + temp.x + 1][col_shift + temp.y] >= kMaxGrainsPerCell) {
            new_topple.Push({static_cast<int64_t>(temp.x + 1), temp.y});
        }

        if (temp.x + row_shift == down_border) down_border--;
        if (temp.y + col_shift == left_border) left_border--;
        if (temp.y + col_shift == right_border - 1) right_border++;
        if (temp.x + row_shift == up_border - 1) up_border++;

        if (beach[row_shift + temp.x][col_shift + temp.y] >= kMaxGrainsPerCell)
            new_topple.Push(temp);
    }

    while (!new_topple.Empty()) {
        topple.Push(new_topple.Pop());
    }

    return (!topple.Empty());
}

void SandPile::EnlargeBeach() {
    capacity_n *= kEnlargeCoefficient;
    capacity_m *= kEnlargeCoefficient;
    int64_t row_delta = capacity_n / (2 * kEnlargeCoefficient), col_delta = capacity_m / (2 * kEnlargeCoefficient);
    uint64_t** new_beach = new uint64_t* [capacity_n];
    for (int64_t k = 0; k < capacity_n; ++k) new_beach[k] = new uint64_t[capacity_m];
    for (int64_t i = 0; i < capacity_n; ++i) {
        for (int64_t j = 0; j < capacity_m; ++j) {
            new_beach[i][j] = static_cast<uint64_t>(0);
        }
    }
    for (int64_t i = down_border; i < up_border; ++i) {
        for (int64_t j = left_border; j < right_border; ++j) {
            new_beach[row_delta + i][col_delta + j] = beach[i][j];
        }
    }

    delete[] beach;
    beach = new_beach;
    row_shift += row_delta;
    col_shift += col_delta;
    left_border += col_delta;
    down_border += row_delta;
    up_border += row_delta;
    right_border += col_delta;
}

struct Color {
    uint8_t r, g, b;

    Color() : r(0), g(0), b(0) {};

    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {};
};

const Color WHITE = Color(255, 255, 255);
const Color GREEN = Color(0, 128, 0);
const Color YELLOW = Color(255, 255, 0);
const Color PURPLE = Color(139, 0, 255);
const Color BLACK = Color(0, 0, 0);

void SandPile::SaveBmp(const char* path, uint64_t iterations) {
    char* new_path = new char[kMaxDigitAmount + sizeof(char)];
    sprintf(new_path, "%s\\%I64d.bmp", path, iterations);

    std::ofstream file;
    file.open(new_path, std::ios::out | std::ios::binary);

    int64_t width = static_cast<int64_t>(right_border - left_border);
    int64_t height = static_cast<int64_t>(up_border - down_border);

    uint8_t padding = (kBitsAlignment - (width) % kBitsAlignment) % kBitsAlignment;
    uint64_t full_width = static_cast<uint64_t>(width + padding);

    uint64_t file_size = kFileHeaderSize + kFileHeaderInfo + kPaletteSize + (full_width * height) / 2;
    uint8_t file_header[kFileHeaderSize] = {};
    uint8_t info_header[kFileHeaderInfo] = {};

    file_header[0] = 'B';
    file_header[1] = 'M';
    // setting size
    file_header[2] = file_size;
    file_header[3] = file_size >> kBitsAlignment;
    file_header[4] = file_size >> (kBitsAlignment * 2);
    file_header[5] = file_size >> (kBitsAlignment * 3);
    // reserved
    file_header[6] = 0;
    file_header[7] = 0;
    file_header[8] = 0;
    file_header[9] = 0;
    // number of bytes from start of file to the first byte of pixel data
    file_header[10] = kFileHeaderSize + kFileHeaderInfo + kPaletteSize;
    file_header[11] = 0;
    file_header[12] = 0;
    file_header[13] = 0;
    // information_header size
    info_header[0] = kFileHeaderInfo;
    info_header[1] = 0;
    info_header[2] = 0;
    info_header[3] = 0;
    // width of image
    info_header[4] = width;
    info_header[5] = width >> kBitsAlignment;
    info_header[6] = width >> (2 * kBitsAlignment);
    info_header[7] = width >> (3 * kBitsAlignment);
    // height of image
    info_header[8] = height;
    info_header[9] = height >> kBitsAlignment;
    info_header[10] = height >> (2 * kBitsAlignment);
    info_header[11] = height >> (3 * kBitsAlignment);
    // planes
    info_header[12] = 1;
    info_header[13] = 0;
    // bits per pixel
    info_header[14] = kBitsPerPixel;
    info_header[15] = 0;
    // total colors: white, green, yellow, purple, black
    info_header[32] = kTotalColors;
    info_header[33] = 0;
    info_header[34] = 0;
    info_header[35] = 0;

    uint8_t colorPalette[kPaletteSize] = {};
    //0 - white
    colorPalette[0] = WHITE.b;
    colorPalette[1] = WHITE.g;
    colorPalette[2] = WHITE.r;
    colorPalette[3] = 0;
    //1 - green
    colorPalette[4] = GREEN.b;
    colorPalette[5] = GREEN.g;
    colorPalette[6] = GREEN.r;
    colorPalette[7] = 0;
    //2 - yellow
    colorPalette[8] = YELLOW.b;
    colorPalette[9] = YELLOW.g;
    colorPalette[10] = YELLOW.r;
    colorPalette[11] = 0;
    //3 - purple
    colorPalette[12] = PURPLE.b;
    colorPalette[13] = PURPLE.g;
    colorPalette[14] = PURPLE.r;
    colorPalette[15] = 0;
    //4 - black
    colorPalette[16] = BLACK.b;
    colorPalette[17] = BLACK.g;
    colorPalette[18] = BLACK.r;
    colorPalette[19] = 0;

    file.write(reinterpret_cast<char*>(file_header), kFileHeaderSize);
    file.write(reinterpret_cast<char*>(info_header), kFileHeaderInfo);
    file.write(reinterpret_cast<char*>(colorPalette), kPaletteSize);

    uint64_t first_pixel, second_pixel;
    uint8_t total_color;
    for (int64_t x = up_border - 1; x >= down_border; --x) {
        for (int64_t y = left_border; y < left_border + full_width; y += 2) {
            if (y >= right_border || y >= capacity_m) {
                first_pixel = second_pixel = 0;
            } else if (y + 1 >= right_border) {
                first_pixel = beach[x][y];
                second_pixel = 0;
            } else {
                first_pixel = beach[x][y];
                second_pixel = beach[x][y + 1];
            }
            if (first_pixel > kMaxGrainsPerCell) first_pixel = kMaxGrainsPerCell;
            if (second_pixel > kMaxGrainsPerCell) second_pixel = kMaxGrainsPerCell;

            total_color = (static_cast<uint8_t>(first_pixel) << kBitsPerPixel) + static_cast<uint8_t>(second_pixel);

            file << total_color;
        }
    }
    file.close();
}


#endif //LABWORK3_ENGOZDEV_SANDPILE_H