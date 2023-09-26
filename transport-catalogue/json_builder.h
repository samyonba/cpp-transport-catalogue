#pragma once

#include "json.h"

#include <string>
#include <optional>

namespace json {

	class Builder;
	class DictItemContext;
	class ArrayItemContext;

	class KeyItemContext {
	public:
		KeyItemContext(Builder& builder)
			: builder_(builder) {}
		DictItemContext Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();

	private:
		Builder& builder_;
	};

	class DictItemContext {
	public:
		DictItemContext(Builder& builder)
			: builder_(builder) {}

		KeyItemContext Key(std::string key);
		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder)
			: builder_(builder) {}

		ArrayItemContext Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

	private:
		Builder& builder_;
	};

	class Builder {
	public:
		KeyItemContext Key(std::string str);
		Builder& Value(Node value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();

	private:
		std::optional<Node> root_;
		std::vector<Node> key_buffer_;
		std::vector<Node*> nodes_stack_;
	};
}
