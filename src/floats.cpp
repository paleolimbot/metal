
#include <cpp11.hpp>
using namespace cpp11;

sexp cpp_floats_from_lgl(logicals x) {
  sexp result_sexp = safe[Rf_allocVector](INTSXP, x.size());
  float* result = reinterpret_cast<float*>(INTEGER(result_sexp));

  R_xlen_t i = 0;
  for (auto el : x) {
    if (el == NA_LOGICAL) {
      result[i++] = NAN;
    } else {
      result[i++] = static_cast<float>(static_cast<int>(el));
    }
  }

  return result_sexp;
}

sexp cpp_floats_from_int(integers x) {
  sexp result_sexp = safe[Rf_allocVector](INTSXP, x.size());
  float* result = reinterpret_cast<float*>(INTEGER(result_sexp));

  R_xlen_t i = 0;
  for (auto el : x) {
    if (el == NA_INTEGER) {
      result[i++] = NAN;
    } else {
      result[i++] = el;
    }
  }

  return result_sexp;
}

sexp cpp_floats_from_dbl(doubles x) {
  sexp result_sexp = safe[Rf_allocVector](INTSXP, x.size());
  float* result = reinterpret_cast<float*>(INTEGER(result_sexp));

  R_xlen_t i = 0;
  for (auto el : x) {
    result[i++] = el;
  }

  return result_sexp;
}

[[cpp11::register]] sexp cpp_floats(double size, double fill) {
  sexp result_sexp = safe[Rf_allocVector](INTSXP, size);
  if (!ISNA(fill)) {
    float* result = reinterpret_cast<float*>(INTEGER(result_sexp));
    R_xlen_t size_xlen = size;
    for (R_xlen_t i = 0; i < size_xlen; i++) {
      result[i] = fill;
    }
  }

  result_sexp.attr("class") = {"mtl_floats", "vctrs_vctr"};
  return result_sexp;
}

[[cpp11::register]] sexp cpp_as_floats(sexp x) {
  if (Rf_isObject(x)) {
    return R_NilValue;
  }

  sexp result;
  switch (TYPEOF(x)) {
    case LGLSXP:
      result = cpp_floats_from_lgl((SEXP)x);
      break;
    case INTSXP:
      result = cpp_floats_from_int((SEXP)x);
      break;
    case REALSXP:
      result = cpp_floats_from_dbl((SEXP)x);
      break;
    default:
      return R_NilValue;
  }

  result.attr("class") = {"mtl_floats", "vctrs_vctr"};
  return result;
}

[[cpp11::register]] logicals cpp_from_floats_lgl(sexp floats_sexp) {
  float* floats = reinterpret_cast<float*>(INTEGER(floats_sexp));
  writable::logicals result(Rf_xlength(floats_sexp));
  for (R_xlen_t i = 0; i < result.size(); i++) {
    if (std::isnan(floats[i])) {
      result[i] = NA_LOGICAL;
    } else {
      result[i] = static_cast<int>(floats[i]);
    }
  }
  return result;
}

[[cpp11::register]] integers cpp_from_floats_int(sexp floats_sexp) {
  float* floats = reinterpret_cast<float*>(INTEGER(floats_sexp));
  writable::integers result(Rf_xlength(floats_sexp));
  for (R_xlen_t i = 0; i < result.size(); i++) {
    if (std::isnan(floats[i])) {
      result[i] = NA_INTEGER;
    } else {
      result[i] = static_cast<int>(floats[i]);
    }
  }
  return result;
}

[[cpp11::register]] doubles cpp_from_floats_dbl(sexp floats_sexp) {
  float* floats = reinterpret_cast<float*>(INTEGER(floats_sexp));
  writable::doubles result(Rf_xlength(floats_sexp));
  for (R_xlen_t i = 0; i < result.size(); i++) {
    result[i] = floats[i];
  }
  return result;
}


