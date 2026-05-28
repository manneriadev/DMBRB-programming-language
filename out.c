#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int32_t x;
    int32_t y;
} math__Point;

void math__Point__translate(math__Point* self, int32_t dx, int32_t dy) {
    self->x = (self->x + dx);
    self->y = (self->y + dy);
    return;

}

int32_t math__Point__get_x(math__Point* self) {
    return self->x;

}

int32_t math__sum(int32_t a, int32_t b) {
    return (a + b);

}

int32_t math__fact(int32_t n) {
    int32_t result = 1;
    int32_t i = 1;
    while ((i <= n)) {
        result = (result * i);
        i = (i + 1);
    }
    return result;

}

int32_t math__max(int32_t a, int32_t b) {
    return ((a > b) ? a : b);

}

int32_t math__abs_val(int32_t x) {
    return ((x < 0) ? (-x) : x);

}

int32_t x = 10;
int32_t y = 20;
int32_t* global_ptr = NULL;
void swap(int32_t* a, int32_t* b) {
    int32_t tmp = (*a);
    (*a) = (*b);
    (*b) = tmp;
    return;

}

int32_t sum_array(int32_t arr[5], int32_t n) {
    int32_t result = 0;
    int32_t i = 0;
    while ((i < n)) {
        result = (result + arr[i]);
        i = (i + 1);
    }
    return result;

}

int32_t main() {
    int32_t a = 10;
    int32_t b = 20;
    int32_t c = math__sum(a, b);
    int32_t d = math__fact(5);
    int64_t e = (((a + b) * c) - d);
    if ((e > 0)) {
        int32_t r = math__sum(e, 1);
    } else {
        int32_t r = math__sum(0, 1);
    }
    if ((e == 0)) {
        int32_t z = 0;
    }
    else if ((e == 10)) {
        int32_t z = 10;
    }
    else {
        int32_t z = 99;
    }
    int32_t i = 0;
    for (int64_t i = 0; i < 10; i++) {
        if ((i == 5)) {
            break;

        }
        int32_t t = math__sum(i, 1);
    }
    while ((a > 0)) {
        a = (a - 1);
        if ((a == 5)) {
            continue;

        }
    }
    int32_t m = math__max(c, d);
    int flag = ((m > 0) ? 1 : 0);
    int32_t bits = 0;
    int32_t mask = 15;
    bits = (bits | mask);
    bits = (bits & 7);
    bits = (bits ^ 3);
    bits = (bits << 1);
    bits = (bits >> 1);
    bits = (~bits);
    double f = 3.14;
    int32_t fi = ((int32_t)(f));
    int32_t ix = 42;
    double fx = ((double)(ix));
    int32_t opt;
    opt = 42;
    ;
    int32_t px = 100;
    int32_t* ptr = (&px);
    int32_t deref_val = (*ptr);
    (*ptr) = 200;
    int32_t* null_ptr = NULL;
    int32_t arr_val = 5;
    int32_t* arr_ptr = (&arr_val);
    int32_t shifted = (*(arr_ptr + 0));
    int32_t p = 3;
    int32_t q = 7;
    swap((&p), (&q));
    global_ptr = (&px);
    int32_t from_global = (*global_ptr);
    if ((c > 0)) {
        if ((d > 0)) {
            int32_t nested = math__sum(c, d);
        } else {
            int32_t nested = 0;
        }
    } else {
        int32_t nested = (-1);
    }
    int32_t outer = 3;
    while ((outer > 0)) {
        int32_t inner = 3;
        while ((inner > 0)) {
            inner = (inner - 1);
        }
        outer = (outer - 1);
    }
    for (int64_t i = 0; i < 20; i += 2) {
        int32_t stepped = math__sum(i, 0);
    }
    int32_t neg = (-5);
    int32_t abs_result = math__abs_val(neg);
    int32_t max_result = math__max(abs_result, d);
    int32_t comp = 10;
    comp += 5;
    comp -= 3;
    comp *= 2;
    comp /= 4;
    comp %= 3;
    const double PI = 3.14159;
    return 0;

}

