#include <limits.h>
#include <stdio.h>

extern float accumulate_runtime(int n, float x);
extern float reference(int n, float x);

// Range annotations do not change execution in the frontend-only comparison.
float set_range(float value, double min, double max, double precision) {
  return value;
}

int main(void) {
  const int bounds[] = {INT_MIN, -4, -1, 0, 1, 4, 17, 100, 101, 1000};
  const float inputs[] = {0.5f, 1.0f, 1.5f};
  for (unsigned i = 0; i < sizeof(bounds) / sizeof(bounds[0]); ++i) {
    for (unsigned j = 0; j < sizeof(inputs) / sizeof(inputs[0]); ++j) {
      float expected = reference(bounds[i], inputs[j]);
      float actual = accumulate_runtime(bounds[i], inputs[j]);
      if (actual != expected) {
        fprintf(stderr, "n=%d x=%g: expected %g, got %g\n", bounds[i],
                inputs[j], expected, actual);
        return 1;
      }
    }
  }
  return 0;
}
