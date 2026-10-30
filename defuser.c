#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t first = (high + 2u * low) ^ 0x3ebbu;
    uint32_t second = 4u * low + (middle ^ 0xc25cu);
    uint32_t answer = (3u * second) ^ (first + 0x469au);
    return write_answer(out, cap, &answer, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = 2u * ((uint32_t)(challenge >> 43) & 15u) + 4u;
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t fragment = (uint32_t)(challenge >> (22u + 3u * (i - 1u))) & 31u;
        values[i] = 5u * values[i - 1u] + (fragment ^ (i + 1u));
    }
    return write_answer(out, cap, values, 6);
}

static uint32_t phase3_value(uint32_t selector, uint32_t x) {
    switch (selector) {
        case 0: return 3u * x + 0x1b0au;
        case 1: return 5u * x + 0x2fdu;
        case 2: return 5u * x + 0x30ad9u;
        case 3: return (4u * x) ^ 0x2088u;
        case 4: return (x ^ 0x60bcu) + (x >> 2);
        case 5: return (4u * x) ^ 0xbfebu;
        case 6: return 3u * x + 0xa7aau;
        case 7: return (x ^ 0x8275u) + (x >> 1);
        default: return 0;
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t selector = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t values[2] = {selector, phase3_value(selector, x)};
    return write_answer(out, cap, values, 2);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + 0xcbu) ^ salt;
    uint32_t child;
    if (target < middle) {
        child = phase4_walk(target, low, middle - 1u, salt);
        return (3u * child) ^ (middle + salt) ^ 2u;
    }
    child = phase4_walk(target, middle + 1u, high, salt);
    return (3u * child) ^ (middle + salt) ^ 0x19u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t target = (uint32_t)challenge & 127u;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t answer = phase4_walk(target, 0, 127u, salt);
    return write_answer(out, cap, &answer, 1);
}

struct phase5_node {
    int next;
    uint32_t tag;
    uint32_t value;
};

static const struct phase5_node phase5_nodes[12] = {
    {9, 1399u, 15708u},
    {4, 100u, 15737u},
    {10, 1887u, 17130u},
    {1, 2689u, 49663u},
    {6, 2883u, 42131u},
    {0, 1614u, 14186u},
    {7, 2352u, 18482u},
    {5, 611u, 31608u},
    {2, 874u, 15671u},
    {-1, 2106u, 64991u},
    {11, 258u, 47895u},
    {3, 1122u, 33207u}
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const int heads[8] = {8, 2, 10, 11, 3, 1, 4, 6};
    uint32_t values[2];
    int node = heads[(uint32_t)challenge & 7u];
    unsigned visits = 1u + ((uint32_t)(challenge >> 3) & 3u);
    uint32_t value = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0x40b9u;
    for (unsigned i = 0; i < visits; ++i) {
        value = (3u * value + phase5_nodes[node].value) ^ phase5_nodes[node].tag;
        values[1] = phase5_nodes[node].tag;
        node = phase5_nodes[node].next;
    }
    values[0] = value;
    return write_answer(out, cap, values, 2);
}

static uint32_t rotate_right7(uint32_t x) {
    return (x >> 7) | (x << 25);
}

static uint32_t phase6_operation(unsigned operation, uint32_t x) {
    switch (operation) {
        case 0: return ((x << 3) | (x >> 29)) ^ 0xc2fau;
        case 1: return rotate_right7(x);
        case 2: return (x ^ 0x3900u) + 4u * x;
        case 3: return ((x << 5) + 0x3905u) ^ (5u * x);
        case 4: return (x + 0x8a65u) ^ (x >> 3);
        case 5: return (x ^ (x >> 3)) + 0x5a48u;
        default: return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    static const unsigned operations[8] = {5, 2, 1, 0, 2, 4, 3, 3};
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    unsigned first = (unsigned)challenge & 7u;
    unsigned second = (unsigned)(challenge >> 3) & 7u;
    x = phase6_operation(operations[first], x) ^ 0x6d14u;
    x = phase6_operation(operations[second], x) + 0x3b26u;
    return write_answer(out, cap, &x, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t selector = (uint32_t)context & 3u;
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t second = rotate_right7(challenge_fold ^ context_fold) ^ 0x23a3u;
    second ^= (selector * 0x5805u) ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (context_fold + selector + 0x7ba6u + (challenge_fold >> 7));
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        selector, second, third);
    return size < 0 || (size_t)size >= cap;
}
