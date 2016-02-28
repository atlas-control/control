// $Id$
/**
 * @file AthContainers/test/AuxTypeVector_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for AuxTypeVector.
 */


#undef NDEBUG


#include "AthContainers/tools/AuxTypeVector.h"
#include <iostream>
#include <cassert>


template <class T>
T makeT(int x=0) { return T(x); }

bool makeT(int x=0) { return (x&1) != 0; }


template <class T>
void test_vector()
{
  SG::AuxTypeVector<T>* vconcrete = new SG::AuxTypeVector<T> (10, 20);
  SG::IAuxTypeVector* v = vconcrete;
  T* ptr = reinterpret_cast<T*> (v->toPtr());
  ptr[0] = makeT(1);
  ptr[1] = makeT(2);
  assert (v->size() == 10);

  assert (vconcrete->vec()[0] == makeT(1));
  assert (vconcrete->vec()[1] == makeT(2));
  assert (&vconcrete->vec() == v->toVector());

  v->reserve (50);
  ptr = reinterpret_cast<T*> (v->toPtr());
  v->resize (40);
  T* ptr2 = reinterpret_cast<T*> (v->toPtr());
  assert (ptr == ptr2);
  assert (ptr[0] == makeT(1));
  assert (ptr[1] == makeT(2));
  assert (v->size() == 40);

  v->shift (1, 1);
  assert (ptr[0] == makeT(1));
  assert (ptr[1] == makeT());
  assert (ptr[2] == makeT(2));

  ptr[1] = makeT(20);
  v->shift (1, -1);
  assert (ptr[0] == makeT(20));
  assert (ptr[1] == makeT(2));

  SG::IAuxTypeVector* v2 = new SG::AuxTypeVector<T> (10, 20);
  ptr2 = reinterpret_cast<T*> (v2->toPtr());
  SG::AuxTypeVector<T>::copy (ptr2, 0, ptr, 1);
  SG::AuxTypeVector<T>::copy (ptr2, 1, ptr, 0);
  assert (ptr2[0] == makeT(2));
  assert (ptr2[1] == makeT(20));

  ptr2[0] = makeT(10);
  ptr2[1] = makeT(11);

  SG::AuxTypeVector<T>::swap (ptr2, 0, ptr, 1);
  assert (ptr[0] == makeT(20));
  assert (ptr[1] == makeT(10));
  assert (ptr2[0] == makeT(2));
  assert (ptr2[1] == makeT(11));

  SG::AuxTypeVector<T>::clear (ptr2, 0);
  assert (ptr2[0] == makeT());
  assert (ptr2[1] == makeT(11));

  SG::IAuxTypeVector* v3 = v->clone();
  assert (v3->size() == v->size());
  T* ptr3 = reinterpret_cast<T*> (v3->toPtr());
  for (size_t i = 0; i < v->size(); i++)
    assert (ptr[i] == ptr3[i]);

  v->resize (0);
  assert (v->toPtr() == 0);

  if (typeid(T) == typeid(bool))
    assert (v->objType() == &typeid(std::vector<char>));
  else
    assert (v->objType() == &typeid(std::vector<T>));

  delete v;
  delete v2;
  delete v3;
}


void test1()
{
  std::cout << "test1\n";
  test_vector<int>();
  test_vector<bool>();
  test_vector<float>();
}


class TestContainer
  : public std::vector<int>, public SG::IAuxSetOption
{
public:
  virtual bool setOption (const SG::AuxDataOption& option) 
  { lastopt = option; return true; }

  static SG::AuxDataOption lastopt;
};


SG::AuxDataOption TestContainer::lastopt ("", 0);


// test setOption
void test2()
{
  std::cout << "test2\n";

  SG::AuxTypeVector<int>* v1 = new SG::AuxTypeVector<int> (10, 20);
  assert (!v1->setOption (SG::AuxDataOption ("opt", 1)));

  SG::AuxTypeVector<int, TestContainer>* v2 =
    new SG::AuxTypeVector<int, TestContainer> (10, 20);
  assert (v2->setOption (SG::AuxDataOption ("opt", 1)));
  assert (TestContainer::lastopt.name() == "opt");
  assert (TestContainer::lastopt.intVal() == 1);
}


// test toPacked
void test3()
{
  std::cout << "test3\n";
  SG::AuxTypeVector<int>* v1 = new SG::AuxTypeVector<int> (0, 0);
  v1->vec().push_back(1);
  v1->vec().push_back(2);

  assert (v1->objType() == &typeid(std::vector<int>));

  void* ptr = v1->toPtr();
  int* iptr = reinterpret_cast<int*>(ptr);
  assert (v1->size() == 2);
  assert (iptr[0] == 1);
  assert (iptr[1] == 2);

  SG::IAuxTypeVector* v2 = v1->toPacked();
  assert (v2 != 0);
  assert (ptr == v2->toPtr());
  assert (v2->size() == 2);
  assert (iptr[0] == 1);
  assert (iptr[1] == 2);
  assert (v1->size() == 0);
  assert (v1->toPtr() == 0);

  assert (v2->objType() == &typeid(SG::PackedContainer<int>));
  SG::PackedContainer<int>* pptr =
    reinterpret_cast<SG::PackedContainer<int>*> (v2->toVector());
  assert (typeid(*pptr) == typeid(SG::PackedContainer<int>));

  SG::AuxTypeVector<std::string>* v3 = new SG::AuxTypeVector<std::string> (0, 0);
  v3->vec().push_back("1");
  v3->vec().push_back("2");

  assert (v3->toPacked() == 0);
  assert (v3->size() == 2);
}


int main()
{
  test1();
  test2();
  test3();
  return 0;
}
