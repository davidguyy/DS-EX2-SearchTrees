// by 11227205 資訊二乙 劉至嘉 & 11027214 楊碕萍.
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>

// DC doesn't support it as of 2025/3/28 :(
// #include <print>

#include <charconv>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace ex2 {

// inspired (copied) from absl::StatusCode
// go to https://abseil.io/docs/cpp/guides/status-codes for documentation
enum struct StatusCode : int {
  kOk = 0,
  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
};

constexpr std::string_view kInputPrefix = "input";
constexpr std::string_view kInputSuffix = ".txt";
constexpr std::string_view kCancelString = "input0.txt";

constexpr std::string_view kPrompt =
    "*** Search Tree Utilities **\n"
    "* 0. QUIT                  *\n"
    "* 1. Build 2-3 tree        *\n"
    "* 2. Build AVL tree        *\n"
    "*************************************\n"
    "Input a choice(0, 1, 2): ";

namespace utils {

std::vector<std::string_view> Split(const std::string_view string,
                                    const std::string_view delimiter) {
  std::vector<std::string_view> tokens;
  for (auto&& token : std::views::split(string, delimiter)) {
    tokens.push_back(static_cast<std::string_view>(token));
  }
  return tokens;
}

void EraseCommaAndQuotation(std::string& s) {
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (*it == ',' || *it == '\"') {
      s.erase(it);
    }
  }
}

/// @return returns 0 if the scanned input cannot be converted to int
int ScanInterger() {
  int input;
  std::cin >> input;
  if (std::cin.fail()) {
    return 0;  // terminates the program
  }
  return input;
}

std::string ScanFileName(const std::string_view prefix,
                         const std::string_view suffix) {
  std::cout << std::format("\nInput a file number ([0] Quit): ");
  std::string file_name;
  std::cin >> file_name;

  file_name.insert(0, prefix);
  file_name.append(suffix);
  return file_name;
}

}  // namespace utils

namespace graduate {

struct Info {
  std::string school_name;
  std::string department_name;
  std::string day_or_night_type;
  std::string level;
  int serial_number = 0;
  int student_amount = 0;
};

std::expected<std::vector<Info>, ex2::StatusCode> MakeList(
    const std::string& file_name) {
  std::ifstream file(file_name);

  if (!file.is_open()) {
    return std::unexpected{ex2::StatusCode::kNotFound};
  }

  auto skip_first_x_lines = [](std::istream& in, const int x) {
    assert(x > 0);
    for (int i = 0; i < x; i++) {
      in.ignore(10000, '\n');
    }
  };

  skip_first_x_lines(file, 3);

  int serial_number = 0;
  auto make_info =
      [&serial_number](const std::vector<std::string_view>& tokens) -> Info {
    enum TokensTable : size_t {
      kSchoolId = 0,
      kSchoolName,
      kDepartmentId,
      kDepartmentName,
      kDayOrNightType,
      kLevel,
      kStudentAmount,
      kTeacherAmount,
      kGraduateAmount,
      kCityName,
      kSchoolType
    };

    ++serial_number;

    auto string_view_to_int = [](std::string_view str) -> int {
      int result = 0;
      std::from_chars(str.data(), str.data() + str.size(), result);
      return result;
    };

    return Info{.school_name = std::string{tokens[kSchoolName]},
                .department_name = std::string{tokens[kDepartmentName]},
                .day_or_night_type = std::string{tokens[kDayOrNightType]},
                .level = std::string{tokens[kLevel]},
                .serial_number = serial_number,
                .student_amount = string_view_to_int(tokens[kStudentAmount])};
  };

  std::string line;
  std::vector<Info> data;
  constexpr std::string_view kDelimiter = "\t";
  while (std::getline(file, line)) {
    data.push_back(make_info(utils::Split(line, kDelimiter)));
  }

  if (data.empty()) [[unlikely]] {
    return std::unexpected{ex2::StatusCode::kFailedPrecondition};
  }

  return data;
}

class AvlTree {
 public:
  struct Node {
    std::vector<int> data;
    std::string key;
    Node* left = nullptr;
    Node* right = nullptr;
    int height = 1;

    bool operator>(const Node& other) const { return key > other.key; }

    bool operator<(const Node& other) const { return key < other.key; }
  };
  using NodePointer = Node*;

  [[nodiscard]] NodePointer MakeNode(const int data, const std::string& key) {
    return new Node{{data}, key};
  }

  AvlTree() = default;

  AvlTree(const AvlTree&) = delete;
  AvlTree& operator=(const AvlTree&) = delete;

  AvlTree(AvlTree&& other) noexcept : root_{other.root_} {
    other.root_ = nullptr;
  }
  AvlTree& operator=(AvlTree&& other) noexcept {
    Clear(root_);
    root_ = other.root_;
    other.root_ = nullptr;
    return *this;
  }

  ~AvlTree() noexcept { Clear(root_); }

  void Clear() noexcept { Clear(root_); }

  void insert(const Info& val) { insert(root_, val); }
  void insert(NodePointer& node, const Info& val) {
    const auto& key = val.department_name;

    if (!node) {
      node = MakeNode(val.serial_number, key);
      return;
    }

    if (key < node->key) {
      insert(node->left, val);
    } else if (key > node->key) {
      insert(node->right, val);
    } else {
      node->data.push_back(val.serial_number);
      return;
    }

    node->height = GetHeight(node);

    const int balance = GetBalanceFactor(node);

    if (balance > 1) {
      if (key < node->left->key) {
        RightRotate(node);
      } else if (key > node->left->key) {
        LeftRotate(node->left);
        RightRotate(node);
      }
    } else if (balance < -1) {
      if (key > node->right->key) {
        LeftRotate(node);
      } else if (key < node->right->key) {
        RightRotate(node->right);
        LeftRotate(node);
      }
    }
  }

  void Insert(const Info& val) {
    auto new_node = MakeNode(val.serial_number, val.department_name);
    if (!root_) [[unlikely]] {
      root_ = new_node;
      return;
    }

    auto current_node = root_;
    auto parent_node = root_;

    while (current_node) {
      current_node->height = GetHeight(current_node);
      parent_node = current_node;

      const bool current_key_is_equal =
          (current_node->key == val.department_name);
      if (current_key_is_equal) {
        current_node->data.push_back(val.serial_number);
        break;
      }

      const bool current_key_is_larger =
          (current_node->key > val.department_name);
      if (current_key_is_larger) {
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }

    const bool parent_key_is_larger = (parent_node->key > val.department_name);
    if (parent_key_is_larger) {
      parent_node->left = new_node;
    } else {
      parent_node->right = new_node;
    }

    parent_node->height = GetHeight(parent_node);

    const int balance_factor = GetBalanceFactor(parent_node);

    if (balance_factor > 1 && new_node->key < parent_node->left->key) {
      RightRotate(parent_node);
    } else if (balance_factor > 1 && new_node->key > parent_node->left->key) {
      LeftRotate(parent_node->left);
      RightRotate(parent_node);
    } else if (balance_factor < -1 && new_node->key < parent_node->left->key) {
      RightRotate(parent_node->right);
      LeftRotate(parent_node);
    } else if (balance_factor < -1 && new_node->key > parent_node->left->key) {
      LeftRotate(parent_node);
    }
  }

  const std::vector<int> GetRoot() const { return root_->data; }

 private:
  void Clear(NodePointer& current) noexcept {
    if (current) {
      Clear(current->left);
      Clear(current->right);
      delete current;
      current = nullptr;
    }
  }

  static int GetHeight(const NodePointer ptr) {
    if (!ptr) {
      return 0;
    }

    auto get_height_with_check = [](const NodePointer ptr) -> int {
      if (!ptr) {
        return 0;
      }
      return ptr->height;
    };

    return std::max(get_height_with_check(ptr->left),
                    get_height_with_check(ptr->right)) +
           1;
  }

  static void LeftRotate(NodePointer& ptr) {
    NodePointer right = ptr->right;
    NodePointer right_left = right->left;

    right->left = ptr;
    ptr->right = right_left;

    ptr->height = GetHeight(ptr);
    right->height = GetHeight(right);

    ptr = right;
  }

  static void RightRotate(NodePointer& ptr) {
    NodePointer left = ptr->left;
    NodePointer left_right = left->right;

    left->right = ptr;
    ptr->left = left_right;

    ptr->height = GetHeight(ptr);
    left->height = GetHeight(left);

    ptr = left;
  }

  static int GetBalanceFactor(const NodePointer ptr) {
    if (!ptr) {
      return 0;
    }
    return GetHeight(ptr->left) - GetHeight(ptr->right);
  }

  NodePointer root_ = nullptr;
};

}  // namespace graduate

class SearchTreeUtility {
 public:
  StatusCode ExecuteCommand(const int command) {
    switch (command) {
      case 0: {
        return StatusCode::kCancelled;
      }
      case 1: {
        list_.clear();
        auto file_name = utils::ScanFileName(kInputPrefix, kInputSuffix);
        auto result = LoadFile(file_name);

        auto not_ok_and_not_cancelled = [](const StatusCode s) -> bool {
          return (s != StatusCode::kOk && s != StatusCode::kCancelled);
        };

        while (not_ok_and_not_cancelled(result)) {
          std::cout << std::format("\n### {} does not exist! ###\n\n",
                                   file_name);

          file_name = utils::ScanFileName(kInputPrefix, kInputSuffix);
          result = LoadFile(file_name);
        }

        MakeTwoThreeTree();
        break;
      }
      case 2: {
        if (!list_.empty()) {
          MakeAvlTree();
        } else {
          std::cout << std::format("### Choose 1 first. ###\n\n");
        }
        break;
      }
      default: {
        return StatusCode::kUnimplemented;
        std::cout << std::format("Command does not exist!\n\n");
      }
    }
    return StatusCode::kOk;
  }

 private:
  StatusCode LoadFile(const std::string& file_name) {
    if (file_name == kCancelString) {
      return StatusCode::kCancelled;
    }

    if (auto list = graduate::MakeList(file_name); list.has_value()) {
      list_ = list.value();
    } else [[unlikely]] {
      return StatusCode::kFailedPrecondition;
    }
    return StatusCode::kOk;
  }

  // TODO: implement trees
  void MakeTwoThreeTree() {}
  void MakeAvlTree() {
    graduate::AvlTree tree;

    auto insert = [&tree](const graduate::Info& val) { tree.insert(val); };

    std::ranges::for_each(list_, insert);

    auto root_data = tree.GetRoot();
    PrintResults(root_data);
  }

  void PrintResults(const std::vector<int>& range) const {
    for (int i = 0; i < range.size(); ++i) {
      const auto& current = list_[range[i] - 1];
      std::cout << std::format("{}: [{}] {}, {}, {}, {}, {}\n", i + 1, range[i],
                               current.school_name, current.department_name,
                               current.day_or_night_type, current.level,
                               current.student_amount);
    }
    std::cout << '\n';
  }

 private:
  std::vector<graduate::Info> list_;
};

}  // namespace ex2

int main() {
  using namespace ex2;

  SearchTreeUtility search_utility;
  // TODO: change from scan integer to getline
  for (StatusCode status = StatusCode::kUnknown;
       status != StatusCode::kCancelled;
       status = search_utility.ExecuteCommand(utils::ScanInterger())) {
    std::cout << std::format(kPrompt);
  }
}