#ifndef IVANP_HED_EXPR_HH
#define IVANP_HED_EXPR_HH

#include <vector>
#include <functional>
#include <iosfwd>

#include <boost/regex.hpp>

#include "shared_str.hh"

struct regex_flags {
  bool s : 1; // select (drop if mismatch)
  bool i : 1; // invert selection and matching
  int  m : 6; // match index
  regex_flags(): s(0), i(0), m(0) { }
};

template <typename T> struct flags;

class TH1;

template <> struct flags<TH1> {
  enum field { none, g, n, t, x, y, z, l, d, f };
  enum add_f { no_add, prepend, append };
  static constexpr unsigned nfields = 9;
  add_f add  : 2; // prepend or append
  unsigned   : 0; // start a new byte
  field from : 4; // field being read
  field to   : 4; // field being set
  int from_i : 8; // field version, python index sign convention
  flags(): add(no_add), from(none), to(none), from_i(-1) { }
  // operator overloading for enums doesn't work for bit fields in GCC
  // https://stackoverflow.com/q/46086830/2640636
};
std::ostream& operator<<(std::ostream&, flags<TH1>::field);
std::ostream& operator<<(std::ostream&, const flags<TH1>&);

struct basic_expr: regex_flags {
  boost::regex re;
  shared_str sub;

  basic_expr(const char*& str); // parsing constructor
  shared_str operator()(shared_str) const; // apply regex

private:
  virtual void assign_flags(const char*& str) = 0;
  virtual void assign_exprs(const char*& str) = 0;
  virtual void assign_fcn(void*,void*) = 0;
};

template <typename T>
struct expr final: flags<T>, basic_expr {
  // TODO: put fcn and exprs in a union
  std::function<void(T*)> fcn;
  std::vector<expr> exprs;

  expr(const char*& str): flags<T>(), basic_expr(str) { }

private:
  void assign_flags(const char*& str) override;
  void assign_exprs(const char*& str) override;
  void assign_fcn(void*,void*) override;
};

using hist_expr = expr<TH1>;

#endif
