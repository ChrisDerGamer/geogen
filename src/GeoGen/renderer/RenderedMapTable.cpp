#include "RenderedMapTable.hpp"
#include "../genlib/HeightMap.hpp"

using namespace std;
using namespace geogen;
using namespace renderer;
using namespace genlib;

bool RenderedMapTable::RemoveItem(String name)
{
	iterator it = this->table.find(name);
	if (it == this->table.end())
	{
		return false;
	}

	delete it->second;
	this->table.erase(it);

	return true;
}

void RenderedMapTable::Clear()
{
	for (iterator it = this->Begin(); it != this->End(); it++)
	{
		delete it->second;
	}

	this->table.clear();
}
