#include <limits.h>
#include <stdio.h>

extern float accumulate_strided(int n, float x);
extern float reference_strided(int n, float x);
extern float accumulate_fixed_stride(float x);
extern float reference_fixed_stride(float x);

// Range annotations are identities in the frontend-only execution test.
float set_range(float value, double min, double max, double precision) {
  return value;
}

int main(void) {
  const int bounds[] = {INT_MIN, -4, -1, 0, 1, 2, 3, 4, 5, 6,
                        7, 8, 9, 10, 17, 100, 101, 1000};
  const float inputs[] = {0.5f, 1.0f, 1.5f};
  for (unsigned j = 0; j < sizeof(inputs) / sizeof(inputs[0]); ++j) {
    float expected_fixed = 3.0f * inputs[j];
    if (accumulate_fixed_stride(inputs[j]) != expected_fixed ||
        reference_fixed_stride(inputs[j]) != expected_fixed) {
      fprintf(stderr, "fixed loop: expected three additions for x=%g\n",
              inputs[j]);
      return 1;
    }
    for (unsigned i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i) {
      float expected = reference_strided(bounds[i], inputs[j]);
      float actual = accumulate_strided(bounds[i], inputs[j]);
      if (actual != expected) {
        fprintf(stderr, "n=%d x=%g: expected %g, got %g\n", bounds[i],
                inputs[j], expected, actual);
        return 1;
      }
    }
  }
  return 0;
}
