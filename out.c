#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int32_t pool[300];
int32_t pool_size = 0;
int32_t nval(int32_t i) {
    return pool[(i * 3)];

}

int32_t nleft(int32_t i) {
    return pool[((i * 3) + 1)];

}

int32_t nright(int32_t i) {
    return pool[((i * 3) + 2)];

}

void set_val(int32_t i, int32_t v) {
    pool[(i * 3)] = v;
}

void set_left(int32_t i, int32_t v) {
    pool[((i * 3) + 1)] = v;
}

void set_right(int32_t i, int32_t v) {
    pool[((i * 3) + 2)] = v;
}

int32_t new_node(int32_t v) {
    int32_t idx = pool_size;
    set_val(idx, v);
    set_left(idx, (-1));
    set_right(idx, (-1));
    pool_size = (pool_size + 1);
    return idx;

}

int32_t insert(int32_t root, int32_t v) {
    if ((root == (-1))) {
        return new_node(v);

    }
    if ((v < nval(root))) {
        set_left(root, insert(nleft(root), v));
    }
    if ((v > nval(root))) {
        set_right(root, insert(nright(root), v));
    }
    return root;

}

int32_t search(int32_t root, int32_t v) {
    if ((root == (-1))) {
        return 0;

    }
    if ((v == nval(root))) {
        return 1;

    }
    if ((v < nval(root))) {
        return search(nleft(root), v);

    }
    return search(nright(root), v);

}

int32_t find_min(int32_t root) {
    int32_t cur = root;
    while ((nleft(cur) != (-1))) {
        cur = nleft(cur);
    }
    return cur;

}

int32_t delete(int32_t root, int32_t v) {
    if ((root == (-1))) {
        return (-1);

    }
    if ((v < nval(root))) {
        set_left(root, delete(nleft(root), v));
        return root;

    }
    if ((v > nval(root))) {
        set_right(root, delete(nright(root), v));
        return root;

    }
    if ((nleft(root) == (-1))) {
        return nright(root);

    }
    if ((nright(root) == (-1))) {
        return nleft(root);

    }
    int32_t m = find_min(nright(root));
    set_val(root, nval(m));
    set_right(root, delete(nright(root), nval(m)));
    return root;

}

void inorder(int32_t root) {
    if ((root == (-1))) {
        return;

    }
    inorder(nleft(root));
    printf("%d ", nval(root));
    inorder(nright(root));
}

int32_t height(int32_t root) {
    if ((root == (-1))) {
        return 0;

    }
    int32_t lh = height(nleft(root));
    int32_t rh = height(nright(root));
    int32_t h = ((lh > rh) ? lh : rh);
    return (h + 1);

}

void visual_print_tree(int32_t root, int32_t space, int32_t value) {
    if ((root == (-1))) {
        return;

    }
    if ((nright(root) != (-1))) {
        visual_print_tree(nright(root), (space + 5), 2);
    }
    int32_t i = 0;
    while ((i < space)) {
        printf(" ");
        i = (i + 1);
    }
    if ((value == 2)) {
        printf("/---%d\r\n", nval(root));
    }
    if ((value == 1)) {
        printf("\\---%d\r\n", nval(root));
    }
    if ((value == 0)) {
        printf("%d\r\n", nval(root));
    }
    if ((nleft(root) != (-1))) {
        visual_print_tree(nleft(root), (space + 5), 1);
    }
}

int32_t main() {
    int32_t root = (-1);
    root = insert(root, 50);
    root = insert(root, 30);
    root = insert(root, 70);
    root = insert(root, 20);
    root = insert(root, 40);
    root = insert(root, 60);
    root = insert(root, 80);
    root = insert(root, 10);
    root = insert(root, 35);
    printf("inorder:  ");
    inorder(root);
    printf("\r\n");
    printf("height:   %d\r\n", height(root));
    printf("search 40: %d\r\n", search(root, 40));
    printf("search 99: %d\r\n", search(root, 99));
    printf("\r\n");
    visual_print_tree(root, 0, 0);
    printf("\r\n");
    root = delete(root, 30);
    printf("after delete 30:\r\n");
    visual_print_tree(root, 0, 0);
    printf("\r\n");
    root = delete(root, 80);
    printf("after delete 80:\r\n");
    visual_print_tree(root, 0, 0);
    printf("\r\n");
    root = delete(root, 50);
    printf("after delete 50:\r\n");
    visual_print_tree(root, 0, 0);
    return 0;

}

