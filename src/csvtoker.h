#pragma once

#include <string>
#include <vector>

struct CsvToker {

  /**
   * Parses each column out of line and places in dest.
   *
   * @param The CSV line to parse.  Caller ensures that line is a single
   * line, rather than the entire contents of a file.
   *
   * @returns number of items in dest on return.
   */
  static int parse(const std::string &line, std::vector<std::string> &dest)
  {
    bool quotes = false;
    bool has_embedded_quote = false;
    const char *p=line.data();
    const char *end=p + line.size();
    const char *start=p;

    while (p < end) {
      switch (*p++) {
        case ',':
          if (!quotes) {
            dest.push_back(trim(start,p,has_embedded_quote));
            start=p;
            has_embedded_quote = false;
          }
          break;

        case '"':
          if (quotes && (p < end && *p == '"')) {
            has_embedded_quote = true;
            p++;
          } else {
            quotes = !quotes;
          }
          break;

        default:
          break;
      }
    }
    if (p >= start) {
      dest.push_back(trim(start,p,has_embedded_quote));
    }
    return dest.size();
  }


  /**
   * Removes whitespace padding and double-quotes.
   * returns trimmed string
   */
  static std::string trim(const char *start, const char *end, bool has_embedded_quote=false)
  {
    bool is_quoted = false;

    if (end <= start) {
      return "";
    }

    while(start < end) {
      if (*start == ' ') {
        start++;
        continue;
      } else if (*start == '"') {
        start++;
        is_quoted = true;
      }
      break;
    }

    // should be at comma delimiter or end

    end--;
    while(end >= start) {
      if (*end == ',' || *end == ' ') {
        end--;
        continue;
      } else if (*end == '"') {
        end--;
      }
      break;
    }

    if (has_embedded_quote) {
      return remove_embedded_quotes(start,end+1);
    }

    return std::string(start, end+1);
  }

  static std::string remove_embedded_quotes(const char *start, const char *end) {
    std::string retval;
    size_t len = end-start;
    {
      char *buf = new char[len+1];
      char *p=buf;
      while (start < end) {
        *p++ = *start++;
        if (*start == '"') start++;
      }
      *p=0;
      retval = std::string(buf);
      delete [] buf;
    }
    return retval;
  }
};
