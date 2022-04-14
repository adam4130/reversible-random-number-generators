#include "wrapper.h"

#include <algorithm>
#include <iterator>

namespace reverse {

// Reversible uniform real generator

UniformRNG<double>* uniform_real_create(double a, double b) {
  return new UniformRNG<double>(a, b);
}

void uniform_real_destroy(UniformRNG<double>* rng) {
  delete rng;
}

void uniform_real_seed(UniformRNG<double>* rng, unsigned long long sd) {
  rng->seed(sd);
}

double uniform_real_next(UniformRNG<double>* rng) {
  return rng->next();
}

double uniform_real_previous(UniformRNG<double>* rng) {
  return rng->previous();
}

void uniform_real_next_array(UniformRNG<double>* rng, double arr[], size_t n) {
  std::generate(arr, arr + n, [rng] { return rng->next(); });
}

void uniform_real_previous_array(UniformRNG<double>* rng, double arr[], size_t n) {
  std::generate(std::make_reverse_iterator(arr + n),
                std::make_reverse_iterator(arr), [rng] { return rng->previous(); });
}

// Reversible uniform integer generator

UniformRNG<int>* uniform_int_create(int a, int b) {
  return new UniformRNG<int>(a, b);
}

void uniform_int_destroy(UniformRNG<int>* rng) {
  delete rng;
}

void uniform_int_seed(UniformRNG<int>* rng, unsigned long long sd) {
  rng->seed(sd);
}

int uniform_int_next(UniformRNG<int>* rng) {
  return rng->next();
}

int uniform_int_previous(UniformRNG<int>* rng) {
  return rng->previous();
}

void uniform_int_next_array(UniformRNG<int>* rng, int arr[], size_t n) {
  std::generate(arr, arr + n, [rng] { return rng->next(); });
}

void uniform_int_previous_array(UniformRNG<int>* rng, int arr[], size_t n) {
  std::generate(std::make_reverse_iterator(arr + n),
                std::make_reverse_iterator(arr), [rng] { return rng->previous(); });
}

// Reversible normal generator

NormalRNG<double>* normal_create(double mean, double stddev) {
  return new NormalRNG<double>(mean, stddev);
}

void normal_destroy(NormalRNG<double>* rng) {
  delete rng;
}

void normal_seed(NormalRNG<double>* rng, unsigned long long sd) {
  rng->seed(sd);
}

double normal_next(NormalRNG<double>* rng) {
  return rng->next();
}

double normal_previous(NormalRNG<double>* rng) {
  return rng->previous();
}

void normal_next_array(NormalRNG<double>* rng, double arr[], size_t n) {
  std::generate(arr, arr + n, [rng] { return rng->next(); });
}

void normal_previous_array(NormalRNG<double>* rng, double arr[], size_t n) {
  std::generate(std::make_reverse_iterator(arr + n),
                std::make_reverse_iterator(arr), [rng] { return rng->previous(); });
}

// Reversible exponential generator

ExponentialRNG<double>* exponential_create(double lambda) {
  return new ExponentialRNG<double>(lambda);
}

void exponential_destroy(ExponentialRNG<double>* rng) {
  delete rng;
}

void exponential_seed(ExponentialRNG<double>* rng, unsigned long long sd) {
  rng->seed(sd);
}

double exponential_next(ExponentialRNG<double>* rng) {
  return rng->next();
}

double exponential_previous(ExponentialRNG<double>* rng) {
  return rng->previous();
}

void exponential_next_array(ExponentialRNG<double>* rng, double arr[], size_t n) {
  std::generate(arr, arr + n, [rng] { return rng->next(); });
}

void exponential_previous_array(ExponentialRNG<double>* rng, double arr[], size_t n) {
  std::generate(std::make_reverse_iterator(arr + n),
                std::make_reverse_iterator(arr), [rng] { return rng->previous(); });
}

} // namespace reverse
