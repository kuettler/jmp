#include <gmp.h>
#include <janet.h>

static Janet cfun_mpz_compare(int32_t argc, Janet *argv);
static Janet cfun_mpz_add(int32_t argc, Janet *argv);
static Janet cfun_mpz_sub(int32_t argc, Janet *argv);
static Janet cfun_mpz_subi(int32_t argc, Janet *argv);
static Janet cfun_mpz_mul(int32_t argc, Janet *argv);
static Janet cfun_mpz_div(int32_t argc, Janet *argv);
static Janet cfun_mpz_divi(int32_t argc, Janet *argv);
static Janet cfun_mpz_divf(int32_t argc, Janet *argv);
static Janet cfun_mpz_divfi(int32_t argc, Janet *argv);
static Janet cfun_mpz_rem(int32_t argc, Janet *argv);
static Janet cfun_mpz_remi(int32_t argc, Janet *argv);
static Janet cfun_mpz_and(int32_t argc, Janet *argv);
static Janet cfun_mpz_or(int32_t argc, Janet *argv);
static Janet cfun_mpz_xor(int32_t argc, Janet *argv);
static Janet cfun_mpz_not(int32_t argc, Janet *argv);

static int mpz_gc(void *data, size_t len)
{
    (void) len;
    mpz_ptr mpz = (mpz_ptr)data;
    mpz_clear(mpz);
    return 0;
}

static int mpz_gcmark(void *data, size_t len)
{
    (void) data;
    (void) len;
    return 0;
}

static JanetMethod mpz_methods[] = {
    {"+", cfun_mpz_add},
    {"r+", cfun_mpz_add},
    {"-", cfun_mpz_sub},
    {"r-", cfun_mpz_subi},
    {"*", cfun_mpz_mul},
    {"r*", cfun_mpz_mul},
    {"/", cfun_mpz_div},
    {"r/", cfun_mpz_divi},
    {"div", cfun_mpz_divf},
    {"rdiv", cfun_mpz_divfi},
    {"%", cfun_mpz_rem},
    {"r%", cfun_mpz_remi},
    {"&", cfun_mpz_and},
    {"r&", cfun_mpz_and},
    {"|", cfun_mpz_or},
    {"r|", cfun_mpz_or},
    {"^", cfun_mpz_xor},
    {"r^", cfun_mpz_xor},
    {"~", cfun_mpz_not},
    {"compare", cfun_mpz_compare},
    {NULL, NULL}
};

static int mpz_get(void *p, Janet key, Janet *out) {
    (void) p;
    if (!janet_checktype(key, JANET_KEYWORD))
        return 0;
    return janet_getmethod(janet_unwrap_keyword(key), mpz_methods, out);
}

static void mpz_tostring(void *p, JanetBuffer *buffer) {
    const size_t str_size = 32;
    char str[str_size];
    size_t required_size = gmp_snprintf(str, str_size, "%Zd", (mpz_ptr)p);
    if (required_size < str_size)
    {
        janet_buffer_push_cstring(buffer, str);
    }
    else
    {
        char* str_ptr = (char*)janet_smalloc(required_size + 1);
        gmp_snprintf(str_ptr, required_size + 1, "%Zd", (mpz_ptr)p);
        janet_buffer_push_cstring(buffer, str_ptr);
        janet_sfree(str_ptr);
    }
}

static Janet mpz_next(void *p, Janet key) {
    (void) p;
    return janet_nextmethod(mpz_methods, key);
}

static int mpz_compare(void *p1, void *p2) {
    return mpz_cmp((mpz_ptr)p1, (mpz_ptr)p2);
}

const JanetAbstractType jmp_mpz_type = {
    "jmp/mpz",
    mpz_gc,
    mpz_gcmark,
    mpz_get,
    NULL,
    NULL, // mpz_marshal,
    NULL, // mpz_unmarshal,
    mpz_tostring,
    mpz_compare,
    NULL, // mpz_hash,
    mpz_next,
    JANET_ATEND_NEXT
};

void janet_unwrap_mpz(Janet x, mpz_ptr mpz) {
    mpz_init(mpz);
    switch (janet_type(x)) {
        default:
            break;
        case JANET_NUMBER : {
            double d = janet_unwrap_number(x);
            mpz_set_d(mpz, d);
            return;
        }
        case JANET_STRING: {
            const uint8_t *str = janet_unwrap_string(x);
            if (mpz_set_str(mpz, str, 0) == 0)
                return;
            break;
        }
        case JANET_ABSTRACT: {
            void *abst = janet_unwrap_abstract(x);
            if (janet_abstract_type(abst) == &janet_s64_type)
            {
                mpz_set_si(mpz, *(int64_t *)abst);
                return;
            }
            else if (janet_abstract_type(abst) == &janet_u64_type)
            {
                mpz_set_ui(mpz, *(uint64_t *)abst);
                return;
            }
            break;
        }
    }
    janet_panicf("can not convert %t %q to an integer", x, x);
    return;
}

static Janet cfun_mpz_add(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_add(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_add_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    if (y > 0) {
                        mpz_add_ui(result, box, (uint64_t)y);
                    } else {
                        mpz_sub_ui(result, box, (uint64_t)-y);
                    }
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_add(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_sub(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_sub(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_sub_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    if (y > 0) {
                        mpz_sub_ui(result, box, (uint64_t)y);
                    } else {
                        mpz_add_ui(result, box, (uint64_t)-y);
                    }
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_sub(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_subi(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    switch (janet_type(argv[1])) {
        default:
            janet_panicf("cannot convert %t %q to integer", argv[1], argv[1]);
            break;
        case JANET_NUMBER: {
            mpz_t y;
            mpz_init_set_d(y, janet_unwrap_number(argv[1]));
            mpz_t result;
            mpz_init(result);
            mpz_sub(result, y, box);
            mpz_swap(result, box);
            mpz_clear(result);
            mpz_clear(y);
            break;
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_mul(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_mul(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_mul_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_mul_si(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_mul(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_div(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                double number = janet_unwrap_number(argv[i]);
                if (number == 0) janet_panic("division by zero");
                mpz_t y;
                mpz_init_set_d(y, number);
                mpz_t result;
                mpz_init(result);
                mpz_tdiv_q(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_tdiv_q_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    if (y < 0) {
                        mpz_tdiv_q_ui(result, box, (uint64_t)-y);
                        mpz_neg(box, result);
                    } else {
                        mpz_tdiv_q_ui(result, box, (uint64_t)y);
                        mpz_swap(result, box);
                    }
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    if (mpz_cmp_ui(y, 0) == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_tdiv_q(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_divi(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    switch (janet_type(argv[1])) {
        default:
            janet_panicf("cannot convert %t %q to integer", argv[1], argv[1]);
            break;
        case JANET_NUMBER: {
            double number = janet_unwrap_number(argv[1]);
            if (number == 0) janet_panic("division by zero");
            mpz_t y;
            mpz_init_set_d(y, number);
            mpz_t result;
            mpz_init(result);
            mpz_tdiv_q(result, y, box);
            mpz_swap(result, box);
            mpz_clear(result);
            mpz_clear(y);
            break;
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_divf(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                double number = janet_unwrap_number(argv[i]);
                if (number == 0) janet_panic("division by zero");
                mpz_t y;
                mpz_init_set_d(y, number);
                mpz_t result;
                mpz_init(result);
                mpz_fdiv_q(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_fdiv_q_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    if (y < 0) {
                        mpz_fdiv_q_ui(result, box, (uint64_t)-y);
                        mpz_neg(box, result);
                    } else {
                        mpz_fdiv_q_ui(result, box, (uint64_t)y);
                        mpz_swap(result, box);
                    }
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    if (mpz_cmp_ui(y, 0) == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_fdiv_q(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_divfi(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    switch (janet_type(argv[1])) {
        default:
            janet_panicf("cannot convert %t %q to integer", argv[1], argv[1]);
            break;
        case JANET_NUMBER: {
            double number = janet_unwrap_number(argv[1]);
            if (number == 0) janet_panic("division by zero");
            mpz_t y;
            mpz_init_set_d(y, number);
            mpz_t result;
            mpz_init(result);
            mpz_fdiv_q(result, y, box);
            mpz_swap(result, box);
            mpz_clear(result);
            mpz_clear(y);
            break;
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_rem(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                double number = janet_unwrap_number(argv[i]);
                if (number == 0) janet_panic("division by zero");
                mpz_t y;
                mpz_init_set_d(y, number);
                mpz_t result;
                mpz_init(result);
                mpz_mod(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    uint64_t y = *(uint64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_mod_ui(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    int64_t y = *(int64_t *)abst;
                    if (y == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    if (y < 0) {
                        mpz_mod_ui(result, box, (uint64_t)-y);
                    } else {
                        mpz_mod_ui(result, box, (uint64_t)y);
                    }
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    if (mpz_cmp_ui(y, 0) == 0) janet_panic("division by zero");
                    mpz_t result;
                    mpz_init(result);
                    mpz_mod(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_remi(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    switch (janet_type(argv[1])) {
        default:
            janet_panicf("cannot convert %t %q to integer", argv[1], argv[1]);
            break;
        case JANET_NUMBER: {
            double number = janet_unwrap_number(argv[1]);
            if (number == 0) janet_panic("division by zero");
            mpz_t y;
            mpz_init_set_d(y, number);
            mpz_t result;
            mpz_init(result);
            mpz_mod(result, y, box);
            mpz_swap(result, box);
            mpz_clear(result);
            mpz_clear(y);
            break;
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_and(int32_t argc, Janet *argv)
{
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_and(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    mpz_t y;
                    mpz_init_set_ui(y, *(uint64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_and(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    mpz_t y;
                    mpz_init_set_si(y, *(int64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_and(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_and(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_or(int32_t argc, Janet *argv)
{
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_ior(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    mpz_t y;
                    mpz_init_set_ui(y, *(uint64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_ior(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    mpz_t y;
                    mpz_init_set_si(y, *(int64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_ior(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_ior(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_xor(int32_t argc, Janet *argv)
{
    janet_arity(argc, 2, -1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init_set(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    for (int32_t i = 1; i < argc; i++) {
        switch (janet_type(argv[i])) {
            default:
                janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                break;
            case JANET_NUMBER: {
                mpz_t y;
                mpz_init_set_d(y, janet_unwrap_number(argv[i]));
                mpz_t result;
                mpz_init(result);
                mpz_xor(result, box, y);
                mpz_swap(result, box);
                mpz_clear(result);
                mpz_clear(y);
                break;
            }
            case JANET_ABSTRACT: {
                void *abst = janet_unwrap_abstract(argv[i]);
                if (janet_abstract_type(abst) == &janet_u64_type) {
                    mpz_t y;
                    mpz_init_set_ui(y, *(uint64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_xor(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &janet_s64_type) {
                    mpz_t y;
                    mpz_init_set_si(y, *(int64_t *)abst);
                    mpz_t result;
                    mpz_init(result);
                    mpz_xor(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                    mpz_clear(y);
                } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                    mpz_ptr y = (mpz_ptr)abst;
                    mpz_t result;
                    mpz_init(result);
                    mpz_xor(result, box, y);
                    mpz_swap(result, box);
                    mpz_clear(result);
                } else {
                    janet_panicf("cannot convert %t %q to integer", argv[i], argv[i]);
                }
                break;
            }
        }
    }
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_not(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init(box);
    mpz_com(box, (mpz_ptr)janet_unwrap_abstract(argv[0]));
    return janet_wrap_abstract(box);
}

static Janet cfun_mpz_compare(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    if (janet_type(argv[0]) != JANET_ABSTRACT)
        goto fail;
    void *abst_x = janet_unwrap_abstract(argv[0]);
    if (janet_abstract_type(abst_x) != &jmp_mpz_type)
        goto fail;
    mpz_ptr x = (mpz_ptr)abst_x;
    switch (janet_type(argv[1])) {
        default:
            break;
        case JANET_NUMBER : {
            double y = janet_unwrap_number(argv[1]);
            return janet_wrap_number(mpz_cmp_d(x, y));
        }
        case JANET_ABSTRACT: {
            void *abst = janet_unwrap_abstract(argv[1]);
            if (janet_abstract_type(abst) == &janet_u64_type) {
                uint64_t y = *(uint64_t *)abst;
                return janet_wrap_number(mpz_cmp_ui(x, y));
            } else if (janet_abstract_type(abst) == &janet_s64_type) {
                int64_t y = *(int64_t *)abst;
                return janet_wrap_number(mpz_cmp_si(x, y));
            } else if (janet_abstract_type(abst) == &jmp_mpz_type) {
                mpz_ptr y = (mpz_ptr)abst;
                return janet_wrap_number(mpz_cmp(x, y));
            }
            break;
        }
    }
    return janet_wrap_nil();
fail:
    janet_panic("compare method requires jmp/mpz as first argument");
    return janet_wrap_nil();
}

JANET_FN(cfun_mpz_new,
         "(jmp/mpz value)",
         "Create a boxed integer from a string value.") {
    janet_fixarity(argc, 1);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    janet_unwrap_mpz(argv[0], box);
    return janet_wrap_abstract(box);
}

JANET_FN(cfun_mpz_setbit,
         "(jmp/setbit value index)",
         "Set bit at index in value.") {
    janet_fixarity(argc, 2);
    mpz_ptr value = (mpz_ptr)janet_getabstract(argv, 0, &jmp_mpz_type);
    size_t index = janet_getsize(argv, 1);
    mpz_setbit(value, index);
    return janet_wrap_nil();
}

JANET_FN(cfun_mpz_clrbit,
         "(jmp/clrbit value index)",
         "Clear bit at index in value.") {
    janet_fixarity(argc, 2);
    mpz_ptr value = (mpz_ptr)janet_getabstract(argv, 0, &jmp_mpz_type);
    size_t index = janet_getsize(argv, 1);
    mpz_clrbit(value, index);
    return janet_wrap_nil();
}

JANET_FN(cfun_mpz_combit,
         "(jmp/combit value index)",
         "Complement bit at index in value.") {
    janet_fixarity(argc, 2);
    mpz_ptr value = (mpz_ptr)janet_getabstract(argv, 0, &jmp_mpz_type);
    size_t index = janet_getsize(argv, 1);
    mpz_combit(value, index);
    return janet_wrap_nil();
}

JANET_FN(cfun_mpz_tstbit,
         "(jmp/tstbit value index)",
         "Test bit at index in value.") {
    janet_fixarity(argc, 2);
    mpz_ptr value = (mpz_ptr)janet_getabstract(argv, 0, &jmp_mpz_type);
    size_t index = janet_getsize(argv, 1);
    int result = mpz_tstbit(value, index);
    return janet_wrap_number(result);
}

JANET_FN(cfun_mpz_import,
         "(jmp/import-str str)",
         "Import a number from a byte string.") {
    janet_fixarity(argc, 1);
    const uint8_t *str = janet_getstring(argv, 0);
    mpz_ptr box = janet_abstract(&jmp_mpz_type, sizeof(mpz_t));
    mpz_init(box);
    mpz_import(box, strlen(str), 1, 1, 0, 0, str);
    return janet_wrap_abstract(box);
}

JANET_FN(cfun_mpz_export,
         "(jmp/export-str str)",
         "Export a number to a byte string.") {
    janet_fixarity(argc, 1);
    mpz_ptr value = (mpz_ptr)janet_getabstract(argv, 0, &jmp_mpz_type);
    int numb = 8;
    int count = (mpz_sizeinbase(value, 2) + numb-1) / numb;
    uint8_t *data = (uint8_t*)janet_smalloc(count);
    size_t written = 0;
    mpz_export(data, &written, 1, 1, 0, 0, value);
    const uint8_t *str = janet_string(data, written);
    janet_sfree(data);
    return janet_wrap_string(str);
}

/****************/
/* Module Entry */
/****************/

JANET_MODULE_ENTRY(JanetTable *env) {
    JanetRegExt cfuns[] = {
        JANET_REG("mpz", cfun_mpz_new),
        JANET_REG("setbit", cfun_mpz_setbit),
        JANET_REG("clrbit", cfun_mpz_clrbit),
        JANET_REG("combit", cfun_mpz_combit),
        JANET_REG("tstbit", cfun_mpz_tstbit),
        JANET_REG("import-str", cfun_mpz_import),
        JANET_REG("export-str", cfun_mpz_export),
        JANET_REG_END
    };
    janet_cfuns_ext(env, "jmp", cfuns);
    janet_register_abstract_type(&jmp_mpz_type);
}
