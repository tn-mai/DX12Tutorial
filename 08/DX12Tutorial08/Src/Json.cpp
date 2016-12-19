/**
* @file Json.cpp
*
* JSON�f�[�^���ǂ�����͂���p�[�U.
* �g���\�肪�Ȃ��̂�true, false, null�ɂ͖��Ή�(�Ȃ̂Łu���ǂ��v).
*/
#include "Json.h"

/**
* JSON�p�[�U.
*/
namespace Json {

/**
* �f�t�H���g�R���X�g���N�^.
*
* ���l�^�Ƃ��ď�����.
*/
Value::Value() : type(Type::Number) {
	new (&number) Number(0.0);
}

/**
* �f�X�g���N�^.
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
* �R�s�[�R���X�g���N�^.
*
* @param v �R�s�[���I�u�W�F�N�g.
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
* ������^�Ƃ��ăR���X�g���N�g����.
*
* @param s ������.
*/
Value::Value(const std::string& s) : type(Type::String) { new(&string) String(s); }

/**
* ���l�^�Ƃ��ăR���X�g���N�g����.
*
* @param d ���l.
*/
Value::Value(double d) : type(Type::Number) { new(&number) Number(d); }

/**
* �I�u�W�F�N�g�^�Ƃ��ăR���X�g���N�g����.
*
* @param o �I�u�W�F�N�g.
*/
Value::Value(const Object& o) : type(Type::Object) { new(&object) Object(o); }

/**
* �z��^�Ƃ��ăR���X�g���N�g����.
*
* @param a �z��.
*/
Value::Value(const Array& a) : type(Type::Array) { new(&array) Array(a); }

void SkipSpace(const char*& data);
Value ParseString(const char*& data);
Value ParseNumber(const char*& data);
Value ParseObject(const char*& data);
Value ParseArray(const char*& data);
Value ParseValue(const char*& data);

/**
* �󔒕������X�L�b�v����.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
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
* ���������͂���.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
*
* @return ��������i�[����Value�^�I�u�W�F�N�g.
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
* ���l����͂���.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
*
* @return ���l���i�[����Value�^�I�u�W�F�N�g.
*/
Value ParseNumber(const char*& data)
{
	char* end;
	const double d = strtod(data, &end);
	data = end;
	return Value(d);
}

/**
* JSON�I�u�W�F�N�g����͂���.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
*
* @return JSON�I�u�W�F�N�g���i�[����Value�^�I�u�W�F�N�g.
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
* JSON�z�����͂���.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
*
* @return JSON�z����i�[����Value�^�I�u�W�F�N�g.
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
* ���͕����ɑΉ�����JSON�I�u�W�F�N�g����͂���.
*
* @param data JSON�f�[�^�̉�͈ʒu�������|�C���^.
*
* @return ���͕����ɑΉ�����JSON�I�u�W�F�N�g���i�[����Value�^�I�u�W�F�N�g.
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
* JSON�f�[�^����͂���.
*
* @param data JSON�f�[�^�̉�͊J�n�ʒu�������|�C���^.
*
* @return ���͕����ɑΉ�����JSON�I�u�W�F�N�g���i�[����Value�^�I�u�W�F�N�g.
*/
Value Parse(const char* data)
{
	return ParseValue(data);
}

} // namespace Json
