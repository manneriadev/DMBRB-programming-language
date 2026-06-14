#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

float A = 0;
float B = 0;
float z[1760];
char b[1760];
int32_t main() {
    float i = 0;
    float j = 0;
    int32_t k = 0;
    printf("\x1b[2J");
    while (1) {
        memset(b, 32, 1760);
        memset(z, 0, 7040);
        j = 0;
        while ((j < 6.28)) {
            i = 0;
            while ((i < 6.28)) {
                float c_ = sinf(i);
                float d_ = cosf(j);
                float e_ = sinf(A);
                float f_ = sinf(j);
                float g_ = cosf(A);
                float l_ = cosf(i);
                float m_ = cosf(B);
                float n_ = sinf(B);
                float h = (d_ + 2);
                float D = (1 / ((((c_ * h) * e_) + (f_ * g_)) + 5));
                float t = (((c_ * h) * g_) - (f_ * e_));
                int32_t x = ((int32_t)((40 + ((30 * D) * (((l_ * h) * m_) - (t * n_))))));
                int32_t y = ((int32_t)((12 + ((15 * D) * (((l_ * h) * n_) + (t * m_))))));
                int32_t o = (x + (80 * y));
                int32_t N = ((int32_t)((8 * ((((((f_ * e_) - ((c_ * d_) * g_)) * m_) - ((c_ * d_) * e_)) - (f_ * g_)) - ((l_ * d_) * n_)))));
                if ((22 > y)) {
                    if ((y > 0)) {
                        if ((x > 0)) {
                            if ((80 > x)) {
                                if ((D > z[o])) {
                                    z[o] = D;
                                    int32_t idx = ((N > 0) ? N : 0);
                                    char ramp[13] = ".,-~:;=!*#$@";
                                    b[o] = ramp[idx];
                                }
                            }
                        }
                    }
                }
                i += 0.02;
            }
            j += 0.07;
        }
        printf("\x1b[H");
        k = 0;
        while ((k < 1761)) {
            if (((k % 80) == 0)) {
                putchar(10);
            } else {
                putchar(((int32_t)(b[k])));
            }
            A += 4e-05;
            B += 2e-05;
            k += 1;
        }
        usleep(5000);
    }
    return 0;

}

