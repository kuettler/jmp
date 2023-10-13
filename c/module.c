#include <janet.h>

/***************/
/* C Functions */
/***************/

JANET_FN(cfun_hello_native,
         "(jmp/hello-native)",
         "Evaluate to \"Hello!\". but implemented in C.") {
    janet_fixarity(argc, 0);
    (void) argv;
    return janet_cstringv("Hello!");
}

/****************/
/* Module Entry */
/****************/

JANET_MODULE_ENTRY(JanetTable *env) {
    JanetRegExt cfuns[] = {
        JANET_REG("hello-native", cfun_hello_native),
        JANET_REG_END
    };
    janet_cfuns_ext(env, "jmp", cfuns);
}