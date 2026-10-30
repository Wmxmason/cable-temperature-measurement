#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ntc_curve.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const float resistance_ohm[] = {
    277225.1f, 211512.9f, 157245.9f, 116610.3f, 87428.8f, 66924.8f,
    51815.4f, 40450.5f, 31770.0f, 24935.0f, 19681.9f, 15621.2f,
    12465.4f, 10000.0f, 8064.4f, 6537.5f, 5327.1f, 4383.2f,
    3592.0f, 2972.2f, 2471.8f, 2065.9f, 1735.3f, 1464.8f,
    1242.5f, 1059.0f, 908.3f, 781.6f, 674.4f, 585.8f,
    508.3f, 441.2f, 383.5f, 334.1f, 292.4f, 256.5f, 225.8f
};

static void require(int condition, const char *message)
{
    if (!condition)
    {
        (void)fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void test_tabulated_points_and_monotonicity(void)
{
    size_t i;
    int32_t previous = INT32_MIN;

    for (i = 0U; i < ARRAY_COUNT(resistance_ohm); i++)
    {
        const int32_t expected = (-40 + (int32_t)i * 5) * 100;
        int32_t actual = INT32_MIN;

        require(ntc_curve_temperature(resistance_ohm[i], &actual),
                "tabulated resistance was rejected");
        require(abs(actual - expected) <= 1,
                "tabulated resistance temperature differs by more than 0.01C");
        require(actual > previous,
                "temperature is not strictly monotonic as resistance decreases");
        previous = actual;
    }
}

static void test_interpolation(void)
{
    int32_t actual;

    actual = INT32_MIN;
    require(ntc_curve_temperature((40450.5f + 31770.0f) / 2.0f, &actual),
            "negative near-zero midpoint was rejected");
    require(actual == -250, "negative near-zero midpoint interpolation is incorrect");

    actual = INT32_MIN;
    require(ntc_curve_temperature((31770.0f + 24935.0f) / 2.0f, &actual),
            "positive near-zero midpoint was rejected");
    require(actual == 250, "positive near-zero midpoint interpolation is incorrect");

    actual = INT32_MIN;
    require(ntc_curve_temperature((277225.1f + 211512.9f) / 2.0f, &actual),
            "cold interval midpoint was rejected");
    require(actual == -3750, "cold interval midpoint interpolation is incorrect");
}

static void test_boundaries_and_invalid_inputs(void)
{
    const int32_t sentinel = 123456789;
    int32_t actual;

    actual = sentinel;
    require(ntc_curve_temperature(277225.1f, &actual) && actual == -4000,
            "lower-temperature table boundary is incorrect");
    actual = sentinel;
    require(ntc_curve_temperature(225.8f, &actual) && actual == 14000,
            "upper-temperature table boundary is incorrect");

    actual = sentinel;
    require(!ntc_curve_temperature(300000.0f, &actual) && actual == sentinel,
            "resistance above table must fail without clamping or output mutation");
    actual = sentinel;
    require(!ntc_curve_temperature(200.0f, &actual) && actual == sentinel,
            "resistance below table must fail without clamping or output mutation");
    actual = sentinel;
    require(!ntc_curve_temperature(NAN, &actual) && actual == sentinel,
            "NaN resistance must fail without output mutation");
    actual = sentinel;
    require(!ntc_curve_temperature(INFINITY, &actual) && actual == sentinel,
            "positive infinity resistance must fail without output mutation");
    actual = sentinel;
    require(!ntc_curve_temperature(-INFINITY, &actual) && actual == sentinel,
            "negative infinity resistance must fail without output mutation");
    require(!ntc_curve_temperature(10000.0f, NULL),
            "null output pointer must fail");
}

int main(void)
{
    test_tabulated_points_and_monotonicity();
    test_interpolation();
    test_boundaries_and_invalid_inputs();
    (void)puts("ntc_curve_test: PASS");
    return EXIT_SUCCESS;
}
