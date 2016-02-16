// module_resource.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "neogfx.hpp"
#include "module_resource.hpp"

namespace neogfx
{
	module_resource::module_resource(const std::string& aPath, const void* aData, std::size_t aSize) : 
		iPath(aPath), iData(aData), iSize(aSize)
	{
	}

	const std::string& module_resource::path() const
	{
		return iPath;
	}

	const void* module_resource::data() const
	{
		return iData;
	}

	std::size_t module_resource::size() const
	{
		return iSize;
	}
}