/**
* @file Json.cpp
*
* JSONデータもどきを解析するパーザ.
* 使う予定がないのでtrue, false, nullには未対応(なので「もどき」).
*/
#include "Json.h"

/**
* JSONパーザ.
*/
namespace Json {

/**
* デフォルトコンストラクタ.
*
* 数値型として初期化.
*/
Value::Value() : type(Type::Number) {
	new (&number) Number(0.0);
}

/**
* デストラクタ.
*/
Value::~Value() {
	switch (type) {
	case Type::String: string.~basic_string(); break;
	case Type::Number: break;
	case Type::Object: object.~map(); break;
	case Type::Array: array.~vector(); break;
	}
}

/**
* コピーコンストラクタ.
*
* @param v コピー元オブジェクト.
*/
Value::Value(const Value& v) {
	type = v.type;
	switch (type) {
	case Type::String: new(&string) String(v.string); break;
	case Type::Number: new(&number) Number(v.number); break;
	case Type::Object: new(&object) Object(v.object); break;
	case Type::Array: new(&array) Array(v.array); break;
	}
}

/**
* 文字列型としてコンストラクトする.
*
* @param s 文字列.
*/
Value::Value(const std::string& s) : type(Type::String) { new(&string) String(s); }

/**
* 数値型としてコンストラクトする.
*
* @param d 数値.
*/
Value::Value(double d) : type(Type::Number) { new(&number) Number(d); }

/**
* オブジェクト型としてコンストラクトする.
*
* @param o オブジェクト.
*/
Value::Value(const Object& o) : type(Type::Object) { new(&object) Object(o); }

/**
* 配列型としてコンストラクトする.
*
* @param a 配列.
*/
Value::Value(const Array& a) : type(Type::Array) { new(&array) Array(a); }

void SkipSpace(const char*& data);
Value ParseString(const char*& data);
Value ParseNumber(const char*& data);
Value ParseObject(const char*& data);
Value ParseArray(const char*& data);
Value ParseValue(const char*& data);

/**
* 空白文字をスキップする.
*
* @param data JSONデータの解析位置を示すポインタ.
*/
void SkipSpace(const char*& data)
{
	for (;; ++data) {
		switch (*data) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			break;
		default:
			goto end;
		}
	}
end:
	return;
}

/**
* 文字列を解析する.
*
* @param data JSONデータの解析位置を示すポインタ.
*
* @return 文字列を格納したValue型オブジェクト.
*/
Value ParseString(const char*& data)
{
	++data; // skip first double quotation.

	std::string s;
	for (; *data != '"'; ++data) {
		s.push_back(static_cast<char>(*data));
	}
	++data; // skip last double quotation.
	return Value(s);
}

/**
* 数値を解析する.
*
* @param data JSONデータの解析位置を示すポインタ.
*
* @return 数値を格納したValue型オブジェクト.
*/
Value ParseNumber(const char*& data)
{
	char* end;
	const double d = strtod(data, &end);
	data = end;
	return Value(d);
}

/**
* JSONオブジェクトを解析する.
*
* @param data JSONデータの解析位置を示すポインタ.
*
* @return JSONオブジェクトを格納したValue型オブジェクト.
*/
Value ParseObject(const char*& data)
{
	++data; // skip first brace.
	SkipSpace(data);
	if (*data == '}') {
		++data;
		return Value(Object());
	}

	Object obj;
	for (;;) {
		const std::string key = ParseString(data).string;
		SkipSpace(data);
		++data; // skip colon.
		SkipSpace(data);
		Value value = ParseValue(data);
		obj.insert(std::make_pair(key, value));

		SkipSpace(data);
		if (*data == '}') {
			++data; // skip last brace.
			break;
		}
		++data; // skip comma.
		SkipSpace(data);
	}
	return Value(obj);
}

/**
* JSON配列を解析する.
*
* @param data JSONデータの解析位置を示すポインタ.
*
* @return JSON配列を格納したValue型オブジェクト.
*/
Value ParseArray(const char*& data)
{
	++data; // skip first bracket.
	SkipSpace(data);
	if (*data == ']') {
		++data;
		return Value(Array());
	}

	Array arr;
	for (;;) {
		Value value = ParseValue(data);
		arr.push_back(value);
		SkipSpace(data);
		if (*data == ']') {
			++data; // skip last bracket.
			break;
		}
		++data; // skip comma.
		SkipSpace(data);
	}
	return Value(arr);
}

/**
* 入力文字に対応するJSONオブジェクトを解析する.
*
* @param data JSONデータの解析位置を示すポインタ.
*
* @return 入力文字に対応するJSONオブジェクトを格納したValue型オブジェクト.
*/
Value ParseValue(const char*& data)
{
	switch (*data) {
	case '"': return ParseString(data);
	case '{': return ParseObject(data);
	case '[': return ParseArray(data);
	default: return ParseNumber(data);
	}
}

/**
* JSONデータを解析する.
*
* @param data JSONデータの解析開始位置を示すポインタ.
*
* @return 入力文字に対応するJSONオブジェクトを格納したValue型オブジェクト.
*/
Value Parse(const char* data)
{
	return ParseValue(data);
}

} // namespace Json
