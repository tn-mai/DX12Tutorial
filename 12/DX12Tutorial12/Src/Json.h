/**
* @file Json.h
*/
#ifndef DX12TUTORIAL_SRC_JSON_H_
#define DX12TUTORIAL_SRC_JSON_H_
#include <string>
#include <vector>
#include <map>

namespace Json {

/**
* Valueが実際に保持しいる型を識別するための列挙型.
*/
enum class Type
{
	String, ///< 文字列型.
	Number, ///< 数値型.
	Object, ///< オブジェクト型.
	Array, ///< 配列型.
};

struct Value;
typedef std::string String;
typedef double Number;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

/**
* JSONの値を格納する汎用型.
*
* typeに対応したメンバ変数にアクセスすることで実際の値が得られる.
* typeと異なるメンバ変数にアクセスした場合の動作は未定義.
*/
struct Value
{
	Value();
	~Value();
	Value(const Value& v);
	Value(const std::string& s);
	Value(double d);
	Value(const Object& o);
	Value(const Array& a);

	/**
	* コピー代入演算子.
	*
	* @param v コピー元オブジェクト.
	*/
	template<typename T>
	Value& operator=(const T& v) {
		(*this).~Value();
		new(this) Value(v);
		return *this;
	}

	Type type;
	union {
		String string;
		Number number;
		Object object;
		Array array;
	};
};

Value Parse(const char* data);

} // namespace Json

#endif // DX12TUTORIAL_SRC_JSON_H_