#pragma once

#include "reverse.h"

namespace reverse {

extern "C" {

UniformRNG<double>* uniform_real_create(double, double);
void uniform_real_destroy(UniformRNG<double>*);
void uniform_real_seed(UniformRNG<double>*, unsigned long long);
double uniform_real_next(UniformRNG<double>*);
double uniform_real_previous(UniformRNG<double>*);
void uniform_real_next_array(UniformRNG<double>*, double[], size_t);
void uniform_real_previous_array(UniformRNG<double>*, double[], size_t);

UniformRNG<int>* uniform_int_create(int, int);
void uniform_int_destroy(UniformRNG<int>*);
void uniform_int_seed(UniformRNG<int>*, unsigned long long);
int uniform_int_next(UniformRNG<int>*);
int uniform_int_previous(UniformRNG<int>*);
void uniform_int_next_array(UniformRNG<int>*, int[], size_t);
void uniform_int_previous_array(UniformRNG<int>*, int[], size_t);

NormalRNG<double>* normal_create(double, double);
void normal_destroy(NormalRNG<double>*);
void normal_seed(NormalRNG<double>*, unsigned long long);
double normal_next(NormalRNG<double>*);
double normal_previous(NormalRNG<double>*);
void normal_next_array(NormalRNG<double>*, double[], size_t);
void normal_previous_array(NormalRNG<double>*, double[], size_t);

ExponentialRNG<double>* exponential_create(double);
void exponential_destroy(ExponentialRNG<double>*);
void exponential_seed(ExponentialRNG<double>*, unsigned long long);
double exponential_next(ExponentialRNG<double>*);
double exponential_previous(ExponentialRNG<double>*);
void exponential_next_array(ExponentialRNG<double>*, double[], size_t);
void exponential_previous_array(ExponentialRNG<double>*, double[], size_t);

} // extern "C"

} // namespace reverse
