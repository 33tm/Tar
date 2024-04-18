#include <filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

struct Header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char modifed[12];
    char checksum[8];
    char type;
    char linkname[100];
};

struct File {
    string name;
    double size;
};

void untar(vector<char> data) {
    if (data.empty()) {
        cout << "Empty file" << endl;
        return;
    }

    vector<File> files;
    vector<char>::iterator base = data.begin();

    while (base != data.end()) {
        Header *header = reinterpret_cast<Header *>(&*base);
        base += 512;

        if (header->type != '0') continue;

        int offset;
        try {
            offset = stoi(header->size, nullptr, 8);
        } catch (invalid_argument) {
            continue;
        };

        filesystem::path path(header->name);
        filesystem::create_directories(path.parent_path());
        files.push_back({ path.filename().string(), ceil(offset / 1024.0 * 100) / 100 });

        vector<char> payload(base, base + offset);
        ofstream out(header->name, ios::binary);
        out.write(payload.data(), payload.size());

        base += offset + 512 - offset % 512;
    }

    unsigned int total = 0;

    for (auto &[name, size] : files) {
        cout << name << right << setw(75 - name.size()) << size << " KB" << endl;
        total += size;
    }

    cout << endl << "Extracted " << total << " KB" << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <tarball>" << endl;
        return 1;
    };

    ifstream tar(argv[1], ios::binary);

    if (!tar) {
        cout << "Failed to open " << argv[1] << endl;
        return 1;
    }

    vector<char> buffer((istreambuf_iterator<char>(tar)), istreambuf_iterator<char>());

    untar(buffer);
}