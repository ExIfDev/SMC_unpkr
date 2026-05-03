#pragma once
class RC4 {
public:
    RC4(const unsigned char* key, int keylen) {
        for (int i = 0; i < 256; i++) {
            S[i] = i;
        }
        int j = 0;
        for (int i = 0; i < 256; i++) {
            j = (j + S[i] + key[i % keylen]) & 255;
            std::swap(S[i], S[j]);
        }
        i = j = 0;
    }

    void process(unsigned char* data, int len) {
        for (int k = 0; k < len; k++) {
            i = (i + 1) & 255;
            j = (j + S[i]) & 255;
            std::swap(S[i], S[j]);
            unsigned char rnd = S[(S[i] + S[j]) & 255];
            data[k] ^= rnd;
        }
    }

private:
    unsigned char S[256];
    int i = 0, j = 0;
};
