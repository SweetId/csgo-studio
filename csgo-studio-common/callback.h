#pragma once

#include <functional>
#include <vector>

template<typename... TArgs>
class Callback
{
public:
	void operator()(TArgs... args)
	{
		for (auto& func : m_funcs)
			func(args...);
	}

	void operator+=(std::function<void(TArgs...)> callback)
	{
		m_funcs.push_back(callback);
	}

private:
	std::vector<std::function<void(TArgs...)>> m_funcs;
};