#ifndef INCLUDE_CCAPI_CPP_CCAPI_DECIMAL_H_
#define INCLUDE_CCAPI_CPP_CCAPI_DECIMAL_H_
#include <string>
#include "ccapi_cpp/ccapi_util_private.h"
namespace ccapi {
// minimalistic just for the purpose of being used as the key of a map
class Decimal CCAPI_FINAL {
 public:
  Decimal() {}
  explicit Decimal(const std::string& originalValue, bool keepTrailingZero = false) {
    if (originalValue.empty()) {
      CCAPI_LOGGER_FATAL("Decimal constructor input value cannot be empty");
    }
    std::string fixedPointValue = originalValue;
    this->sign = true;
    if (fixedPointValue.at(0) == '-') {
      fixedPointValue.erase(0, 1);
      this->sign = false;
    }
    if (fixedPointValue.find("E") != std::string::npos || fixedPointValue.find("e") != std::string::npos) {
      std::vector<std::string> splitted = UtilString::split(fixedPointValue, fixedPointValue.find("E") != std::string::npos ? "E" : "e");
      fixedPointValue = splitted.at(0);
      if (fixedPointValue.find(".") != std::string::npos) {
        fixedPointValue = UtilString::rtrim(UtilString::rtrim(fixedPointValue, "0"), ".");
      }
      auto exponent = splitted.at(1);
      if (exponent.at(0) == '+') {
        exponent.erase(0, 1);
      }
      exponent = UtilString::ltrim(exponent, "0");
      if (exponent.empty()) {
        exponent = "0";
      }
      if (exponent != "0") {
        if (fixedPointValue.find(".") != std::string::npos) {
          std::vector<std::string> splittedByDecimal = UtilString::split(fixedPointValue, ".");
          if (exponent.at(0) != '-') {
            if (std::stoi(exponent) < splittedByDecimal.at(1).length()) {
              fixedPointValue =
                  splittedByDecimal.at(0) + splittedByDecimal.at(1).substr(0, std::stoi(exponent)) + "." + splittedByDecimal.at(1).substr(std::stoi(exponent));
            } else {
              fixedPointValue = splittedByDecimal.at(0) + splittedByDecimal.at(1) + std::string(std::stoi(exponent) - splittedByDecimal.at(1).length(), '0');
            }
          } else {
            fixedPointValue = "0." + std::string(-std::stoi(exponent) - 1, '0') + splittedByDecimal.at(0) + splittedByDecimal.at(1);
          }
        } else {
          if (exponent.at(0) != '-') {
            fixedPointValue += std::string(std::stoi(exponent), '0');
          } else {
            fixedPointValue = "0." + std::string(-std::stoi(exponent) - 1, '0') + fixedPointValue;
          }
        }
      }
    }
    std::vector<std::string> splitted = UtilString::split(fixedPointValue, ".");
    // TODO(cryptochassis): replace with std::from_chars() once upgrade to C++17
    this->before = std::stoull(splitted.at(0));
    if (splitted.size() > 1) {
      this->frac = splitted.at(1);
      if (!keepTrailingZero) {
        this->frac = UtilString::rtrim(this->frac, "0");
      }
    }
  }
  std::string toString() const {
    std::string stringValue;
    if (!this->sign) {
      stringValue += "-";
    }
    stringValue += std::to_string(this->before);
    if (!this->frac.empty()) {
      stringValue += ".";
      stringValue += this->frac;
    }
    return stringValue;
  }
  double toDouble() const { return std::stod(this->toString()); }
  friend bool operator<(const Decimal& l, const Decimal& r) {
    if (l.sign && r.sign) {
      if (l.before < r.before) {
        return true;
      } else if (l.before > r.before) {
        return false;
      } else {
        return l.frac < r.frac;
      }
    } else if (l.sign && !r.sign) {
      return false;
    } else if (!l.sign && r.sign) {
      return true;
    } else {
      Decimal nl = l;
      nl.sign = true;
      Decimal nr = r;
      nr.sign = true;
      return nl > nr;
    }
  }
  friend bool operator>(const Decimal& l, const Decimal& r) { return r < l; }
  friend bool operator<=(const Decimal& l, const Decimal& r) { return !(l > r); }
  friend bool operator>=(const Decimal& l, const Decimal& r) { return !(l < r); }
  friend bool operator==(const Decimal& l, const Decimal& r) { return !(l > r) && !(l < r); }
  friend bool operator!=(const Decimal& l, const Decimal& r) { return !(l == r); }
  Decimal negate() const {
    Decimal o;
    o.before = this->before;
    o.frac = this->frac;
    o.sign = !this->sign;
    return o;
  }
  Decimal add(const Decimal& x) const {
    if (this->sign && x.sign) {
      Decimal o;
      o.sign = true;
      o.before = this->before + x.before;
      if (this->frac.empty()) {
        o.frac = x.frac;
      } else if (x.frac.empty()) {
        o.frac = this->frac;
      } else {
        auto l1 = this->frac.length();
        auto l2 = x.frac.length();
        if (l1 > l2) {
          auto a = std::to_string(std::stoull(this->frac) + std::stoull(UtilString::rightPadTo(x.frac, l1, '0')));
          if (a.length() < l1) {
            a = UtilString::leftPadTo(a, l1, '0');
          }
          if (a.length() == l1) {
            o.frac = UtilString::rtrim(a, "0");
          } else {
            o.frac = UtilString::rtrim(a.substr(a.length() - l1), "0");
            o.before += std::stoull(a.substr(0, a.length() - l1));
          }
        } else if (l1 < l2) {
          auto a = std::to_string(std::stoull(UtilString::rightPadTo(this->frac, l2, '0')) + std::stoull(x.frac));
          if (a.length() < l2) {
            a = UtilString::leftPadTo(a, l2, '0');
          }
          if (a.length() == l2) {
            o.frac = UtilString::rtrim(a, "0");
          } else {
            o.frac = UtilString::rtrim(a.substr(a.length() - l2), "0");
            o.before += std::stoull(a.substr(0, a.length() - l2));
          }
        } else {
          auto a = std::to_string(std::stoull(this->frac) + std::stoull(x.frac));
          if (a.length() < l1) {
            a = UtilString::leftPadTo(a, l1, '0');
          }
          if (a.length() == l1) {
            o.frac = UtilString::rtrim(a, "0");
          } else {
            o.frac = UtilString::rtrim(a.substr(a.length() - l1), "0");
            o.before += std::stoull(a.substr(0, a.length() - l1));
          }
        }
      }
      return o;
    } else if (!this->sign && x.sign) {
      return x.subtract(this->negate());
    } else if (this->sign && !x.sign) {
      return this->subtract(x.negate());
    } else {
      return (this->negate().add(x.negate())).negate();
    }
  }
  Decimal subtract(const Decimal& x) const {
    if (this->sign && x.sign) {
      if (*this > x) {
        Decimal o;
        o.sign = true;
        if (this->frac >= x.frac) {
          o.before = this->before - x.before;
        } else {
          o.before = this->before - 1 - x.before;
        }
        auto l1 = this->frac.length();
        auto l2 = x.frac.length();
        auto lmax = std::max(l1, l2);
        auto a = lmax>0?std::to_string(std::stoull(UtilString::rightPadTo(this->frac, lmax, '0')) +
                                (this->frac >= x.frac ? (unsigned)0 : std::stoull(UtilString::rightPadTo("1", 1 + lmax, '0'))) -
                                std::stoull(UtilString::rightPadTo(x.frac, lmax, '0'))):"";
        if (a.length() < lmax) {
          a = UtilString::leftPadTo(a, lmax, '0');
        }
        o.frac = UtilString::rtrim(a, "0");
        return o;
      } else {
        return x.subtract(*this).negate();
      }
    } else if (!this->sign && x.sign) {
      return x.subtract(this->negate());
    } else if (this->sign && !x.sign) {
      return this->subtract(x.negate());
    } else {
      return x.negate().subtract(this->negate());
    }
  }
  // {-}bbbb.aaaa
  unsigned long long before{};
  std::string frac;
  // false means negative sign needed
  bool sign{};
};
} /* namespace ccapi */
#endif  // INCLUDE_CCAPI_CPP_CCAPI_DECIMAL_H_
