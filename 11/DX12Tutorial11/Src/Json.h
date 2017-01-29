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
* Value�����ۂɕێ�������^�����ʂ��邽�߂̗񋓌^.
*/
enum class Type
{
	String, ///< ������^.
	Number, ///< ���l�^.
	Object, ///< �I�u�W�F�N�g�^.
	Array, ///< �z��^.
};

struct Value;
typedef std::string String;
typedef double Number;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

/**
* JSON�̒l���i�[����ėp�^.
*
* type�ɑΉ����������o�ϐ��ɃA�N�Z�X���邱�ƂŎ��ۂ̒l��������.
* type�ƈقȂ郁���o�ϐ��ɃA�N�Z�X�����ꍇ�̓���͖���`.
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
	* �R�s�[������Z�q.
	*
	* @param v �R�s�[���I�u�W�F�N�g.
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