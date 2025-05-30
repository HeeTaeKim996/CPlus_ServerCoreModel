/*-----------------------------------------
		코드 재작성 안하고 그냥 복붙
-----------------------------------------*/



#pragma once

#include "Types.h"


// static_cast는 안정성이 낮고(형변환 시도후 실패시 오류 발생). dynamic_cast는 캐스트 실패시
// 오류가 아닌 nullptr을 리턴. dynamic_cast는 연산이 느린 단점이 있음
// TypeCast 아래 코드는 런타임이 아닌 컴파일 때 오류가 발생하도록 하는, 안정성? 을 갖춘 형변환
// 을 위한 코드

// ※ 내용이 많이 어려운데. 대충 정리하면 런타임이 아닌 컴파일 때 에러가 발생하도록 만드는 코드
//   -> 런타임에서 수행하는 코드 (for.. ) 를 사용하지 않고, template과 재귀함수 를 사용해,
//      컴파일에서 형변환 에러를 발견하도록 하는 코드.

// ※ 아래처럼 template 을 활용하여 컴파일 때 오류를 내도록 하는 기법의 원리는, 
//   Moderen C++ 책? (옛날책이라 함) 을 공부하는 게 좋다 함	

#pragma region TyeList
template<typename...T>
struct TypeList;
// 전방 선언과 비슷한 역할

template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};


template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};
// 재귀 호출을 위한 역할. TypeList<A, B, C> => TypeList<A, TypeList<B, C>> 


#pragma endregion


#pragma region Length
template<typename T>
struct Length;

template<>
struct Length<TypeList<>>
{
	enum {value = 0};
};

template<typename T, typename ...U>
struct Length<TypeList<T, U...>>
{
	enum { value = 1 + Length<TypeList<U...>>::value };
};

// => Length<TypeList<Mage, Knight, Archer>>::value = (1 + (1 + ( 1 + 0 ) ) ) = 3
#pragma endregion


#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;

template<typename Head, typename...Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};
	 
// => 마찬가지로 재귀호출. 
//  using TL = TyeList<Mage, Knight, Archer>;
//  TypeAt<TL, 2>::Result = TypeAt< TypeList<Knight, Archer>, 1> = TypeAt<Archer, 0> = Archer
#pragma endregion


#pragma region IndexOf
template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum{value = 0};
};


template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum{value = -1};
};


template<typename Head, typename ...Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum{temp =IndexOf<TypeList<Tail...>, T>::value};

public:
	enum { value = (temp == -1) ? -1 : temp + 1 };

};
// => Header에 찾는 T가 있으면 0 반환. T가 TL에 아얘 없다면 -1 반환.
//    둘 다 아닐시, 재귀호출로 index 찾음. temp에서 Head를 잘라서, 나머지로 재귀 호출.
//    재귀 호출로 얻은 temp 값이 -1일시, 1 반환. -1이 아닐시, 재귀 호출에서 역으로 + 1을
//    반복하여, value를 반환. 
// (Header를 자르는 재귀호출에서 Index가 -1 씩 밀리므로. value에서 역으로 +1 로 재귀? 호출하여 반환) 
#pragma endregion


#pragma region Conversion
template<typename From, typename To>
class Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }
	static Big Test(...) { return 0; } // 여기서 ... 는 '아무 타입이나 받는다' 를 의미
	static From MakeFrom() { return 0; }

	// ※ 위 함수의 { } (내용)은 없어도 된다. Conversion은 함수 오버로드의 성질을 이용한 것이기 때문

public:
	enum
	{
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
	};
};

// => 함수의 오버로드 성질을 이용한 코드.
//    예시로, 
//    1) Coversion<Player, Knight>:: exists
//     -> MakeFrom() -> Player. Test(Player) -> Small Test(const Knight&) 호출.
//     -> Small 반환. -> exists = sizeof(Small) == sizeof(Small). -> True 반환.

//    2) Conversion<Knight, Player>:: exists
//     -> MakeFrom() -> Knight. Test(Knight) -> Big Test(...) 호출.
//       ( !! 매개변수의 인자에 업캐스팅 불가)
//     -> Big 반환. -> exists = sizeof(Big) == sizeof(Small) -> False 반환.

//    3) Conversion<Knight, Dog>::exists
//      -> 2)와 같은 절차로 false 반환
#pragma endregion




#pragma region TypeCast

// 아래 s_convert[i][j] 를 for문에서 처리하지 않고, 아래 코드들처럼 길게 재귀호출로 처리하는 이유는, 
// 아래 코드로 만들어져 사용되는 s_convert[i][j]가 런타임이 아닌 컴파일 타임 정보를 기반으로 
// 테이블을 구성하기 위함


template<int32 v>
struct Int2Type
{
	enum{value = v};
};


template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeTable(Int2Type<0>(), Int2Type<0>());
	}

	template<int32 i, int32 j>
	static void MakeTable(Int2Type<i>, Int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		s_convert[i][j] = Conversion<const FromType*, const ToType*>::exists ?
			true : false;
#pragma region FromType*, ToType* 으로 pt 를 넣는 이유
		// - 우선 A*, B*등 pt는 unsigned int의 메모리 주소 정보 뿐만 아닌, A, B등 클래스 정보?도
		//   담고 있다.
		//  -> A*, B* 를 비교해서 A와 B가 상속관계인지도 알 수 있다.

		// - FromType, ToType이 아닌 FromType*, ToType* 인 이유는, 이 코드들은 컴파일 타임 정보
		//   기반으로 변환 가능 유무를 알려주는 기능을 하는데, pt 기반으로 비교를 해야 컴파일 타임
		//   정보 기반으로 비교가 가능하기 때문이라 한다. (From 지피티)
#pragma endregion



		MakeTable(Int2Type<i>(), Int2Type<j + 1>());
	}

	// 위 재귀호출에서 Int2Type<j+1> == Int2Type<length>일 때 호출
	template<int32 i>
	static void MakeTable(Int2Type<i>, Int2Type<length>) 
	{
		MakeTable(Int2Type<i + 1>(), Int2Type<0>());
	}

	// 위 재귀호출에서 Int2Type<i+1> == Int2Type<length> 일 때 호출
	template<int32 j>
	static void MakeTable(Int2Type<length>, Int2Type<j>)
	{
	}


	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[length][length];
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];



template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (!ptr)
	{
		return nullptr;
	}

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
	{
		return static_cast<To>(ptr);
	}

	return nullptr;
}


template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
	{
		return false;
	}

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(
		ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}


// shared_ptr 버전 2 추가

#include "Memory.h"

template<typename To, typename From>
std::shared_ptr<To> TypeCast(std::shared_ptr<From> ptr)
{
	if (!ptr)
	{
		return nullptr;
	}

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
	{
		return std::static_pointer_cast<To>(ptr);
	}

	return nullptr;
}

template<typename To, typename From>
bool CanCast(std::shared_ptr<From> ptr)
{
	if (ptr == nullptr)
	{
		return false;
	}

	using TL = typename From::TL;

	return TypeConversion<TL>::CanConvert(
		ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}

#pragma endregion


#define DECLARE_TL		using TL = TL; int32 _typeId
#define INIT_TL(Type)	_typeId = IndexOf<TL, Type>::value