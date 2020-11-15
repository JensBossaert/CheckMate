// C = Animation*
template<typename C>
class Stack {
  private:
    C* _stack;
    C* _tail;
    int _length = 3;

  public:
    Stack() {
      _tail = _stack = new C[_length];
    }

    ~Stack() {
      delete[] _stack;
    }

    unsigned int size() const {
      return _tail - _stack;
    }

    bool isEmpty() const {
      return _tail == _stack;
    }

    void push(const C &a) {
      if (_tail == _stack + _length) {
        // The allocated space for the stack is insufficient.
        // => reallocate a larger array and copy data
        C* oldStack = _stack;
        C* oldTail = _tail;
        _stack = new C[_length *= 2];
        _tail = _stack;
        for (C* i = oldStack; i != oldTail; ++_tail, ++i)
          *_tail = *i;
        delete[] oldStack;
      }
      *(_tail++) = a;
    }

    C pop() {
      return *(--_tail);
    }

    template<typename T>
    void filter(T filterFunction) {
      C* newTail = _stack;
      for (C* i = _stack; i < _tail; ++i) {
        if (filterFunction(*i)) {
          *(newTail++) = *i;
        }
      }
      _tail = newTail;
    }

    C &operator[](unsigned int i) {
      return _stack[i];
    }

    C* begin() {
      return _stack;
    }

    C* end() {
      return _tail;
    }
};
