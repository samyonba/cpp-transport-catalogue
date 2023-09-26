#include "json_builder.h"
#include <cassert>
#include <stdexcept>

using namespace json;
using namespace std;

KeyItemContext json::Builder::Key(std::string str)
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}

	if (!nodes_stack_.back()->IsDict())
	{
		throw logic_error("Calling the Key method outside the dictionary or immediately after another Key"s);
	}

	key_buffer_.push_back(Node(str));
	nodes_stack_.push_back(&key_buffer_.back());

	return { *this };
}

Builder& json::Builder::Value(Node value)
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}

	if (!(!root_.has_value() || nodes_stack_.back()->IsString() || nodes_stack_.back()->IsArray()))
	{
		throw logic_error("Calling Value anywhere except after the constructor, after the Key, or after the previous element of the array"s);
	}

	if (!root_.has_value())
	{
		root_ = std::move(value);
		return *this;
	}

	if (nodes_stack_.back()->IsArray())
	{
		const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value);
	}
	if (nodes_stack_.back()->IsString())
	{
		const string key = nodes_stack_.back()->AsString();
		nodes_stack_.pop_back();
		key_buffer_.pop_back();
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ key, value });
	};

	return *this;
}

DictItemContext json::Builder::StartDict()
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}
	if (!(!root_.has_value() || nodes_stack_.back()->IsString() || nodes_stack_.back()->IsArray()))
	{
		throw logic_error("Calling StartDict anywhere except after the constructor, after the Key, or after the previous element of the array"s);
	}

	if (!root_.has_value())
	{
		root_ = Dict();
		nodes_stack_.push_back(&root_.value());
		return { *this };
	}

	if (nodes_stack_.back()->IsArray())
	{
		// добавляет элемент последнего открытого массива
		const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
		// добавляет в nodes_stack_ указатель на добавленный Node(Dict);
		nodes_stack_.push_back(&(const_cast<Array&>(nodes_stack_.back()->AsArray()).back()));
	}
	if (nodes_stack_.back()->IsString())
	{
		const string key = nodes_stack_.back()->AsString();
		nodes_stack_.pop_back();
		key_buffer_.pop_back();
		// добавляет элемент в последний открытый словарь
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ key, Dict() });
		// добавляет в nodes_stack_ указатель на добавленный Node(Dict);
		nodes_stack_.push_back(&(const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]));
	};

	return { *this };
}

ArrayItemContext json::Builder::StartArray()
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}
	if (!(!root_.has_value() || nodes_stack_.back()->IsString() || nodes_stack_.back()->IsArray()))
	{
		throw logic_error("Calling StartArray anywhere except after the constructor, after the Key, or after the previous element of the array"s);
	}

	if (!root_.has_value())
	{
		root_ = Array();
		nodes_stack_.push_back(&root_.value());
		return { *this };
	}

	if (nodes_stack_.back()->IsArray())
	{
		// добавляет элемент последнего открытого массива
		const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
		// добавляет в nodes_stack_ указатель на добавленный Node(Array);
		nodes_stack_.push_back(&(const_cast<Array&>(nodes_stack_.back()->AsArray()).back()));
	}
	if (nodes_stack_.back()->IsString())
	{
		const string key = nodes_stack_.back()->AsString();
		nodes_stack_.pop_back();
		key_buffer_.pop_back();
		// добавляет элемент в последний открытый словарь
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ key, Array() });
		// добавляет в nodes_stack_ указатель на добавленный Node(Array);
		nodes_stack_.push_back(&(const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]));
	};

	return { *this };
}

Builder& json::Builder::EndDict()
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}
	if (!nodes_stack_.back()->IsDict())
	{
		throw logic_error("Calling EndDict in the context of another container"s);
	}

	nodes_stack_.pop_back();

	return *this;
}

Builder& json::Builder::EndArray()
{
	if (root_.has_value() && nodes_stack_.empty())
	{
		throw logic_error("Calling any method other than Build when the object is ready"s);
	}
	if (!nodes_stack_.back()->IsArray())
	{
		throw logic_error("Calling EndArray in the context of another container"s);
	}

	nodes_stack_.pop_back();

	return *this;
}

Node json::Builder::Build()
{
	if (!root_.has_value() || !nodes_stack_.empty())
	{
		throw logic_error("Calling the Build method when the described object is not ready"s);
	}

	return root_.value();
}

DictItemContext json::KeyItemContext::Value(Node value)
{
	builder_.Value(value);
	return { builder_ };
}

DictItemContext json::KeyItemContext::StartDict()
{
	builder_.StartDict();
	return { builder_ };
}

ArrayItemContext json::KeyItemContext::StartArray()
{
	builder_.StartArray();
	return { builder_ };
}

KeyItemContext json::DictItemContext::Key(std::string key)
{
	builder_.Key(key);
	return { builder_ };
}

Builder& json::DictItemContext::EndDict()
{
	builder_.EndDict();
	return builder_;
}

ArrayItemContext json::ArrayItemContext::Value(Node value)
{
	builder_.Value(value);
	return { builder_ };
}

DictItemContext json::ArrayItemContext::StartDict()
{
	builder_.StartDict();
	return { builder_ };
}

ArrayItemContext json::ArrayItemContext::StartArray()
{
	builder_.StartArray();
	return { builder_ };
}

Builder& json::ArrayItemContext::EndArray()
{
	builder_.EndArray();
	return builder_;
}
