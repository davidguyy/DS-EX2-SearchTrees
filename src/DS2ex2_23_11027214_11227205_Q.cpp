// by 11227205 資訊二乙 劉至嘉 & 11027214 楊碕萍.
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <span>
#include <stack>

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
    "* 3. Intersection Query    *\n"
    "*************************************\n"
    "Input a choice(0, 1, 2, 3): ";

namespace utils {

std::vector<std::string_view> StrSplit(std::string_view string,
                                       std::string_view delimiter) {
  auto tokens = string | std::ranges::views::split(delimiter);

  return std::vector<std::string_view>{tokens.begin(), tokens.end()};
}

int StrToInt(std::string_view str) noexcept {
  int value = 0;
  std::from_chars(str.data(), str.data() + str.size(), value);
  return value;
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

    return Info{.school_name = std::string{tokens[kSchoolName]},
                .department_name = std::string{tokens[kDepartmentName]},
                .day_or_night_type = std::string{tokens[kDayOrNightType]},
                .level = std::string{tokens[kLevel]},
                .serial_number = serial_number,
                .student_amount = utils::StrToInt(tokens[kStudentAmount])};
  };

  std::string line;
  std::vector<Info> data;
  constexpr std::string_view kDelimiter = "\t";

  while (std::getline(file, line)) {
    utils::EraseCommaAndQuotation(line);
    data.push_back(make_info(utils::StrSplit(line, kDelimiter)));
  }

  if (data.empty()) [[unlikely]] {
    return std::unexpected{ex2::StatusCode::kFailedPrecondition};
  }

  return data;
}

struct Dot {
  Dot(const Info& val) {
    data = {val.serial_number};
    key = val.school_name;
  }

  void Insert(Dot* dot) {
    auto current = this;
    while (current->next != nullptr) {
      current = current->next;
    }

    current->next = dot;
  }

  ~Dot() noexcept { Clear(next); }

  void Clear(Dot*& ptr) noexcept {
    if (ptr) {
      Clear(ptr->next);
      delete ptr;
      ptr = nullptr;
    }
  }

  int data;
  std::string key;
  Dot* next = nullptr;  // record the same dot(school)
};

class Node {
 public:
  Dot* GetDotInNode(int index) {
    if (index > dots.size()) return nullptr;
    return dots[index];
  }  // GetDotInNode()

  void SetParent(Node* parent) { this->parent = parent; }

  bool HasParent() {
    if (parent) return true;
    return false;
  }  // HasParent()

  Node* GetParent() { return this->parent; }

  Node* GetNextNode() {
    if (childs.size() == 0) return nullptr;
    return childs[0];
  }  // GetNextNode()

  // return how many dots in one node
  int GetDotSize() { return dots.size(); }  // GetDotSize()

  int GetChildrenSize() { return childs.size(); }

  void SortChildren() {
    if (childs.size() == 0 || childs.size() == 1) return;
    for (int i = 0; i < childs.size() - 1; ++i) {
      for (int j = 0; j < childs.size() - i - 1; ++j) {
        if (childs[j]->GetDotInNode(0)->key >
            childs[j + 1]->GetDotInNode(0)->key) {
          Node* temp = childs[j];
          childs[j] = childs[j + 1];
          childs[j + 1] = temp;
        }  // if
      }
    }
    /*
    std::sort(dots.begin(), dots.end(), [](Dot* a, Dot* b) {
        return a->GetDotInfo().school_name[0] <
    b->GetDotInfo().school_name[0];
        });
    */
  }

  // sort dots in one node
  void SortDot() {
    if (dots.size() == 0 || dots.size() == 1) return;

    for (int i = 0; i < dots.size() - 1; ++i) {
      for (int j = 0; j < dots.size() - i - 1; ++j) {
        if (dots[j]->key > dots[j + 1]->key) {
          Dot* temp = dots[j];
          dots[j] = dots[j + 1];
          dots[j + 1] = temp;
        }  // if
      }
    }
    /*
    std::sort(dots.begin(), dots.end(), [](Dot* a, Dot* b) {
        return a->GetDotInfo().school_name[0] <
    b->GetDotInfo().school_name[0];
        });
    */
  }  // SortDot()

  // whether node has children or not
  bool NodeHasChildren() {
    if (childs.size() != 0) {
      return true;
    }
    return false;
  }  // NodeHasChildren()

  void InsertChild(Node* child) { this->childs.push_back(child); }

  void InsertDot(Dot* dot) {
    for (int i = 0; i < dots.size(); ++i) {
      if (dots[i]->key == dot->key) {
        dots[i]->Insert(dot);
        return;
      }
    }

    dots.push_back(dot);
    SortDot();
  }  // insertDot()

  Node* GetChildrenAt(int i) { return childs[i]; }

  void SplitDot() {
    /*
     curNode:
             dots:    [10, 20, 28]
             childs: []
    */
    if (parent == nullptr) {
      Node* sibling = new Node();
      Node* newRoot = new Node();
      newRoot->InsertDot(this->dots[1]);

      for (int i = (this->dots.size() / 2) + 1; i < this->dots.size(); ++i) {
        sibling->InsertDot(dots[i]);
      }

      for (int i = this->childs.size() / 2; i < this->childs.size(); ++i) {
        sibling->InsertChild(childs[i]);
        childs[i]->parent = sibling;
      }

      this->dots.erase(dots.begin() + (dots.size() / 2), dots.end());
      this->childs.erase(childs.begin() + (childs.size() / 2), childs.end());

      this->parent = newRoot;
      sibling->parent = newRoot;

      newRoot->InsertChild(this);
      newRoot->InsertChild(sibling);
      newRoot->SortDot();
      newRoot->SortChildren();
      sibling->SortDot();
      sibling->SortChildren();
      return;
    }

    else if (parent != nullptr) {
      Node* sibling = new Node();
      this->parent->InsertDot(dots[1]);
      for (int i = (this->dots.size() / 2) + 1; i < this->dots.size(); ++i) {
        sibling->InsertDot(dots[i]);
      }

      // Copy new half child to new sibling  node
      for (int i = this->childs.size() / 2; i < this->childs.size(); ++i) {
        sibling->InsertChild(childs[i]);
        childs[i]->parent = sibling;
      }

      sibling->SetParent(this->parent);
      this->parent->InsertChild(sibling);
      // Erase the middle and the right sibling
      this->dots.erase(dots.begin() + (dots.size() / 2), dots.end());
      // Erase the right-half child
      this->childs.erase(childs.begin() + (childs.size() / 2), childs.end());

      this->parent->SortDot();
      SortDot();
      sibling->SortDot();

      this->parent->SortChildren();
      SortChildren();
      sibling->SortChildren();
      if (parent->GetDotSize() == 3) parent->SplitDot();
    }

  }  // SplitDot()

  const std::vector<Dot*> GetDots() const { return dots; }

  ~Node() noexcept {
    auto delete_dot = [](Dot*& ptr) {
      delete ptr;
      ptr = nullptr;
    };
    auto delete_node = [](Node*& ptr) {
      delete ptr;
      ptr = nullptr;
    };

    std::ranges::for_each(dots, delete_dot);
    std::ranges::for_each(childs, delete_node);
  }

 private:
  std::vector<Dot*> dots;
  Node* parent = nullptr;
  std::vector<Node*> childs;
};

class TwoThreeTree {
 public:
  int GetHeight() {
    if (root == nullptr) return 0;

    Node* current = root;
    int level = 1;
    while (current->NodeHasChildren()) {
      ++level;
      current = current->GetNextNode();
    }

    return level;
  }

  bool IsEmpty() const { return root == nullptr; }

  // least complex 2-3 tree method
  std::vector<int> GetRootData() const { return GetData(root); }

  std::vector<int> GetData(const Node* node) const {
    const auto& node_data = node->GetDots();
    std::vector<int> results;
    for (const auto& i : node_data) {
      for (auto j = i; j; j = j->next) {
        results.push_back(j->data);
      }
    }
    std::ranges::sort(results);
    return results;
  }

  TwoThreeTree(const std::span<Info>& range) {
    auto insert = [this](const Info& val) { Insert(val); };

    std::ranges::for_each(range, insert);
  }
  TwoThreeTree() = default;

  void Insert(const std::span<Info>& range) {
    auto insert = [this](const Info& val) { Insert(val); };

    std::ranges::for_each(range, insert);
  }

  std::vector<int> Search(std::string_view key) {
    if (key == "*") {
      return TraverseAll();
    }
    auto dot = root;
    if (root == nullptr) {
      return {};
    }  // if()

    Node* current = root;
    std::vector<int> results;

    while (current) {
      auto& node_data = current->GetDots();
      for (const auto& i : node_data) {
        if (key == i->key) {
          for (auto j = i; j; j = j->next) {
            results.push_back(j->data);
          }
        }
      }

      if (!current->NodeHasChildren()) {
        break;
      }

      if (current->GetDotSize() == 1) {
        if (key < current->GetDotInNode(0)->key) {
          current = current->GetChildrenAt(0);
        }

        else {
          current = current->GetChildrenAt(1);
        }
      }

      else if (current->GetDotSize() == 2) {
        if (key < current->GetDotInNode(0)->key) {
          current = current->GetChildrenAt(0);
        }

        else if (key > current->GetDotInNode(0)->key &&
                 key < current->GetDotInNode(1)->key) {
          current = current->GetChildrenAt(1);
        }

        else if (key > current->GetDotInNode(1)->key) {
          current = current->GetChildrenAt(2);
        }
      }
    }

    std::ranges::sort(results);
    return results;
  }

  std::vector<int> TraverseAll() {
    auto result = GetData(root);
    Traverse(root, result);
    return result;
  }

  // remove copy constructor/assignment because we didn't implement deep copy :/
  TwoThreeTree(const TwoThreeTree&) = delete;
  TwoThreeTree& operator=(const TwoThreeTree&) = delete;

  void Clear() noexcept {
    delete root;
    root = nullptr;
  }
  ~TwoThreeTree() noexcept { delete root; }

  void Insert(const ex2::graduate::Info& info) {
    auto dot = new Dot(info);
    if (root == nullptr) {
      Node* node = new Node();
      node->InsertDot(dot);
      root = node;
      return;
    }  // if()

    Node* current = root;
    Node* parents = nullptr;

    while (current->NodeHasChildren()) {
      // Horizontal search
      for (int i = 0; i < current->GetDotSize(); ++i) {
        if (current->GetDotInNode(i)->key == dot->key) {
          current->GetDotInNode(i)->Insert(dot);
          while (current->HasParent()) {
            current = current->GetParent();
          }
          this->root = current;
          return;
        }  // if
      }  // for

      if (current->GetDotSize() == 1) {
        if (dot->key < current->GetDotInNode(0)->key) {
          current = current->GetChildrenAt(0);
        }

        else {
          current = current->GetChildrenAt(1);
        }
      }

      else if (current->GetDotSize() == 2) {
        if (dot->key < current->GetDotInNode(0)->key) {
          current = current->GetChildrenAt(0);
        }

        else if (dot->key > current->GetDotInNode(0)->key &&
                 dot->key < current->GetDotInNode(1)->key) {
          current = current->GetChildrenAt(1);
        }

        else if (dot->key > current->GetDotInNode(1)->key) {
          current = current->GetChildrenAt(2);
        }
      }
    }

    current->InsertDot(dot);
    if (current->GetDotSize() == 3) current->SplitDot();
    current->SortDot();
    while (current->HasParent()) {
      current = current->GetParent();
    }
    this->root = current;

  }  // Insert()
 private:
  void Traverse(Node* node, std::vector<int>& results) {
    if (!node->NodeHasChildren()) {
      return;
    }
    for (int i = 0; i < node->GetChildrenSize(); ++i) {
      Traverse(node->GetChildrenAt(i), results);
      auto vec = GetData(node->GetChildrenAt(i));
      results.insert(results.end(), vec.begin(), vec.end());
    }
  }
  Node* root = nullptr;
};

class AvlTree {
 public:
  struct Node {
    std::vector<int> data;
    std::string key;
    Node* left = nullptr;
    Node* right = nullptr;
    int height = 1;
  };
  using NodePointer = Node*;

  [[nodiscard]] static NodePointer MakeNode(const int data,
                                            const std::string& key) {
    return new Node{{data}, key};
  }

  AvlTree() = default;
  AvlTree(const std::span<Info>& range) {
    std::ranges::for_each(range, [this](const Info& v) { Insert(v); });
  }

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

  bool IsEmpty() const { return root_ == nullptr; }

  void Clear() noexcept { Clear(root_); }

  void Insert(const std::span<Info>& range) {
    std::ranges::for_each(range, [this](const Info& v) { Insert(v); });
  }

  void Insert(const Info& val) { root_ = Insert(root_, val); }

  static NodePointer Insert(const NodePointer node, const Info& val) {
    const auto& key = val.department_name;

    if (!node) [[unlikely]] {
      return MakeNode(val.serial_number, key);
    }

    if (key < node->key) {
      node->left = Insert(node->left, val);
    } else if (key > node->key) {
      node->right = Insert(node->right, val);
    } else {
      node->data.push_back(val.serial_number);
      return node;
    }

    node->height = MaxHeight(node);

    return Rotate(node, key);
  }

  int GetRootHeight() const { return HeightCheck(root_); }

  const std::vector<int> GetRootData() const { return root_->data; }

  const std::vector<int> Search(std::string_view target_key) {
    if (target_key == "*") {
      return Inorder();
    }

    NodePointer current = root_;
    bool key_found = false;
    while (current) {
      if (current->key < target_key) {
        current = current->right;
      } else if (current->key > target_key) {
        current = current->left;
      } else {
        key_found = true;
        break;
      }
    }

    if (!key_found) {
      return {};
    }
    return current->data;
  }

  std::vector<int> Inorder() const {
    if (!root_) {
      return {};
    }

    std::vector<int> result;
    std::stack<NodePointer> stack;
    NodePointer current = root_;
    while (current || !stack.empty()) {
      if (current) {
        stack.push(current);
        current = current->left;
      } else {
        NodePointer previous = stack.top();
        stack.pop();
        result.insert(result.begin(), previous->data.begin(),
                      previous->data.end());
        current = previous->right;
      }
    }
    return result;
  }

 private:
  void Clear(NodePointer& current) noexcept {
    if (current) {
      Clear(current->left);
      Clear(current->right);
      delete current;
      current = nullptr;
    }
  }

  static int HeightCheck(const NodePointer ptr) noexcept {
    if (!ptr) {
      return 0;
    }
    return ptr->height;
  }

  static int MaxHeight(const NodePointer ptr) noexcept {
    assert(ptr);
    return std::max(HeightCheck(ptr->left), HeightCheck(ptr->right)) + 1;
  }

  static int GetBalanceFactor(const NodePointer ptr) {
    assert(ptr);
    return HeightCheck(ptr->left) - HeightCheck(ptr->right);
  }

  static NodePointer LeftRotate(const NodePointer ptr) {
    assert(ptr);
    NodePointer right_child = ptr->right;
    NodePointer right_child_left_sub_tree = right_child->left;

    right_child->left = ptr;
    ptr->right = right_child_left_sub_tree;

    ptr->height = MaxHeight(ptr);
    right_child->height = MaxHeight(right_child);

    return right_child;
  }

  static NodePointer RightRotate(const NodePointer ptr) {
    assert(ptr);
    NodePointer left_child = ptr->left;
    NodePointer left_child_right_sub_tree = left_child->right;

    left_child->right = ptr;
    ptr->left = left_child_right_sub_tree;

    ptr->height = MaxHeight(ptr);
    left_child->height = MaxHeight(left_child);

    return left_child;
  }

  static NodePointer Rotate(const NodePointer node, std::string_view key) {
    const int balance_factor = GetBalanceFactor(node);

    if (balance_factor > 1) {
      if (key < node->left->key) {
        return RightRotate(node);
      } else {
        node->left = LeftRotate(node->left);
        return RightRotate(node);
      }
    } else if (balance_factor < -1) {
      if (key > node->right->key) {
        return LeftRotate(node);
      } else {
        node->right = RightRotate(node->right);
        return LeftRotate(node);
      }
    }
    return node;
  }

 private:
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
          std::cout << std::format("\n### {} does not exist! ###\n", file_name);

          file_name = utils::ScanFileName(kInputPrefix, kInputSuffix);
          result = LoadFile(file_name);
        }

        if (result == StatusCode::kCancelled) [[unlikely]] {
          std::cout << '\n';
          break;
        }

        MakeTwoThreeTree();
        break;
      }
      case 2: {
        if (!list_.empty()) {
          MakeAvlTree();
        } else {
          std::cout << "### Choose 1 first. ###\n\n";
        }
        break;
      }
      case 3: {
        if (two_three_.IsEmpty()) {
          std::cout << "### Choose 1 first. ###\n\n";
          break;
        } else if (avl_.IsEmpty()) {
          std::cout << "### Choose 2 first. ###\n\n";
          break;
        }
        SearchIntersection();
        break;
      }
      default: {
        std::cout << std::format("\nCommand does not exist!\n\n");
        return StatusCode::kUnimplemented;
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
  void MakeTwoThreeTree() {
    two_three_.Clear();
    two_three_.Insert(list_);

    std::cout << std::format("Tree height = {}\n", two_three_.GetHeight());
    PrintNode(two_three_.GetRootData());
  }

  void MakeAvlTree() {
    avl_.Clear();
    avl_.Insert(list_);

    std::cout << std::format("Tree height = {}\n", avl_.GetRootHeight());
    PrintNode(avl_.GetRootData());
  }

  void SearchIntersection() {
    auto school_name = ScanString(ScanOption::kScanSchoolName);
    auto department_name = ScanString(ScanOption::kScanDepartmentName);

    auto two_three_result = two_three_.Search(school_name);
    auto avl_result = avl_.Search(department_name);

    std::ranges::sort(two_three_result);
    std::ranges::sort(avl_result);

    std::vector<int> out;
    std::ranges::set_intersection(two_three_result, avl_result,
                                  std::back_inserter(out));
    PrintNode(out);
  }

  enum struct ScanOption { kScanSchoolName, kScanDepartmentName };
  std::string ScanString(const ScanOption option) {
    std::string result;
    if (option == ScanOption::kScanSchoolName) {
      std::cout << "Enter a college name to search [*]: ";
    } else if (option == ScanOption::kScanDepartmentName) {
      std::cout << "Enter a department name to search [*]: ";
    }
    std::cin >> result;
    return result;
  }

  void PrintNode(const std::vector<int>& range) const {
    for (int i = 0; i < range.size(); ++i) {
      const auto& current = list_[range[i] - 1];
      std::cout << std::format("{}: [{}] {}, {}, {}, {}, {}\n", i + 1, range[i],
                               current.school_name, current.department_name,
                               current.day_or_night_type, current.level,
                               current.student_amount);
    }
    std::cout << "\n\n";
  }

 private:
  std::vector<graduate::Info> list_;
  graduate::TwoThreeTree two_three_;
  graduate::AvlTree avl_;
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