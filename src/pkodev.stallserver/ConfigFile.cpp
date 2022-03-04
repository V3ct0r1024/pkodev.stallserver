#include "ConfigFile.h"
#include "Utils.h"

#include <fstream>

namespace pkodev
{
	// Constructor
	ConfigParam::ConfigParam() :
		data(std::make_pair("", ""))
	{

	}

	// Constructor
	ConfigParam::ConfigParam(const std::string& key_, const std::string& value_) :
		data(std::make_pair(key_, value_))
	{

	}

	// Convert the parameter to int
	int ConfigParam::to_integer() const
	{
		// Conversion result
		int result = 0;

		// Try to convert a string to int
		try
		{
			result = std::stoi(data.second);
		}
		catch (const std::invalid_argument& e)
		{
			throw config_file_exception(
				std::string(
				 "Could not convert parameter '" + data.second + "' to type 'int'!"
					" std::invalid_argument exception: " + e.what()
				)
			);
		}
		catch (const std::out_of_range& e)
		{
			throw config_file_exception(
				std::string(
					"Could not convert parameter '" + data.second + "' to type 'int'!"
					" std::out_of_range exception: " + e.what()
				)
			);
		}

		// Return the conversion result
		return result;
	}

	// Convert the parameter to double
	double ConfigParam::to_double() const
	{
		// Conversion result
		double result = 0;

		// Try to convert a string to double
		try
		{
			result = std::stod(data.second);
		}
		catch (const std::invalid_argument& e)
		{
			throw config_file_exception(
				std::string(
					"Could not convert parameter '" + data.second + "' to type 'double'!"
					" std::invalid_argument exception: " + e.what()
				)
			);
		}
		catch (const std::out_of_range& e)
		{
			throw config_file_exception(
				std::string(
					"Could not convert parameter '" + data.second + "' to type 'double'!"
					" std::out_of_range exception: " + e.what()
				)
			);
		}

		// Return the conversion result
		return result;
	}

	// Convert the parameter to bool
	bool ConfigParam::to_bool() const
	{
		// Get the parameter value in lowercase
		std::string lower = utils::string::lower_case(data.second);

		// Compare the parameter value with 'true', 'false' or '0'
		return (
			( (lower.compare("false") != 0) && (lower.compare("0") != 0) ) 
				|| (lower.compare("true") == 0)
		);
	}
	
	// Convert the parameter to std::string
	const std::string& ConfigParam::to_string() const
	{
		return data.second;
	}


	// Constructor
	ConfigFile::ConfigFile()
	{

	}

	// Destructor 
	ConfigFile::~ConfigFile()
	{

	}

	// Load settings from file
	void ConfigFile::load(const std::string& path)
	{
		// Clear old paramaters
		m_sections.clear();

		// Open the file
		std::ifstream fp(path);

		// Check if file is open
		if (fp.is_open() == false)
		{
			throw config_file_exception(
				std::string("Could not open file '" + utils::file::extract_filename(path) + "' to load program settings!")
			);
		}

		// File line counter
		std::size_t counter = 0;

		// Current line
		std::string line("");

		// Current section
		std::string section("");

		// Read the file line by line
		while (std::getline(fp, line))
		{
			// Increase the line counter
			++counter;

			// Remove whitespaces from the line
			line = utils::string::trim(line);

			// Check that the line is empty
			if (line.empty() == true)
			{
				// Skip empty lines
				continue;
			}

			// Check that the line is commented out
			if ( (line.find("//") == 0) || (line.find("\\\\") == 0) || (line.find("#") == 0) )
			{
				// Skip commented lines
				continue;
			}

			// Look for a comment in the line
			std::size_t pos = line.find_first_of("//\\\\#");
			if (pos != std::string::npos)
			{
				// Remove the comment from the line
				line = utils::string::trim(line.substr(0, pos));
			}

			// Get the line type (section or key = value pair)
			bool is_section = ( (line.front() == '[') && (line.back() == ']') );

			// Check the line type
			if (is_section == true)
			{
				// Get the name of the current section
				section = utils::string::lower_case(
					utils::string::trim(
						line.substr(
							1,
							line.length() - 2
						)
					)
				);
			}
			else
			{
				// Check that the section has been defined
				if (section.empty() == true)
				{
					throw config_file_exception(
						std::string("Syntax error at line " + std::to_string(counter) + ": No sections have been defined yet!")
					);
				}

				// Look for the '=' sign
				pos = line.find('=');

				// Check that the sign is found
				if (pos == std::string::npos)
				{
					throw config_file_exception(
						std::string("Syntax error at line " + std::to_string(counter) + ": Delimiter '=' not found!")
					);
				}

				// Get the key
				std::string key = utils::string::lower_case(
					utils::string::trim(line.substr(0, pos))
				);

				// Get the value
				std::string value = utils::string::trim(line.substr(pos + 1, line.length() - 1));

				// Add a parameter to the section
				m_sections[section].push_back({ key, value });
			}
		}

		// Close the file
		fp.close();
	}

	// Save settings to file
	void ConfigFile::save(const std::string& path)
	{
		// Open the file
		std::ofstream fp(path);

		// Check if file is open
		if (fp.is_open() == false)
		{
			throw config_file_exception(
				std::string("Could not open file '" + utils::file::extract_filename(path) + "' to save program settings!")
			);
		}

		// Loop through the sections
		for (auto const& section : m_sections)
		{
			// Write name of the section
			fp << '[' << section.first << ']' << '\n';

			// Write parameters of the section
			for (auto const& param : section.second)
			{
				fp << param.data.first << " = " << param.data.second << '\n';
			}

			// Write an empty string
			fp << '\n';
		}

		// Close the file
		fp.close();
	}

	// Get the number of parameters in a section with the same key
	std::size_t ConfigFile::param_count(const std::string& section, const std::string& key) const
	{
		// Counter of found parameters
		std::size_t counter = 0;

		// Look for a section in the list
		auto it = m_sections.find(utils::string::lower_case(section));

		// Check that the section is found
		if (it != m_sections.end())
		{
			// Convert the key to lowercase
			std::string key_lower = utils::string::lower_case(key);

			// Go through the list of parameters
			for (auto const& param : it->second)
			{
				// Compare keys
				if (param.data.first.compare(key_lower) == 0)
				{
					// Parameter found, increase the counter
					++counter;
				}
			}
		}

		// Return the number of same parameters in a section
		return counter;
	}

	// Get a parameter
	const ConfigParam& ConfigFile::get(const std::string& section, const std::string& key,
		const std::string& default_value, std::size_t index) const
	{
		// Look for a section in the list
		auto it = m_sections.find(utils::string::lower_case(section));

		// Check that the section is found
		if (it != m_sections.end())
		{
			// Counter of found parameters
			std::size_t counter = 0;

			// Convert the key to lowercase
			std::string key_lower = utils::string::lower_case(key);

			// Go through the list of parameters
			for (auto const& param : it->second)
			{
				// Compare keys
				if (param.data.first.compare(key_lower) == 0)
				{
					// Check the index
					if (index == counter)
					{
						// Return the parameter
						return param;
					}

					// Increase the counter
					++counter;
				}
			}
		}

		// Set the default parameter
		m_null.data = std::make_pair(key, default_value);

		// Return the default parameter
		return m_null;
	}

	// Set a parameter
	std::size_t ConfigFile::set(const std::string& section, const std::string& key,
		const std::string& value, std::size_t index)
	{
		// Convert the section name to lowercase
		std::string section_lower = utils::string::lower_case(section);

		// Add a new parameter
		if (index == UINT_MAX)
		{
			// Get the number of parameters with the specified key in the section
			std::size_t n = param_count(section_lower, key);

			// Check that the number of parameters does not exceed the maximum value
			if (n == (UINT_MAX - 1))
			{
				throw config_file_exception(
					std::string("Can't add new parameter '" + key + "' because reached limit!")
				);
			}

			// Add the parameter to the list
			m_sections[section_lower].push_back({ key, value });

			// Return the index of the new parameter
			return n;
		}
		
		// Edit an existing parameter
		auto it = m_sections.find(section_lower);

		// Check that the section is found
		if (it == m_sections.end())
		{
			throw config_file_exception(
				std::string("Can't edit parameter '" + key + "' because section '" + section + "' not found!")
			);
		}

		// Counter of found parameters
		std::size_t counter = 0;

		// Convert the key to lowercase
		std::string key_lower = utils::string::lower_case(key);

		// Check that the parameter with the specified index exists
		for (auto& param : it->second)
		{
			// Compare keys
			if (param.data.first.compare(key_lower) == 0)
			{
				// Check the index
				if (index == counter)
				{
					// Update the parameter
					param = { key, value };

					// Return the same index
					return index;
				}

				// Increase the counter
				++counter;
			}
		}

		// The parameter with the specified index was not found
		throw config_file_exception(
			std::string("Can't edit parameter '" + key + "' because index '" + std::to_string(index) + "' not found!")
		);
	}

	// Remove a parameter
	void ConfigFile::remove(const std::string& section, 
		const std::string& key, std::size_t index)
	{
		// Look for a section in the list
		auto it = m_sections.find(utils::string::lower_case(section));

		// Check that the section is found
		if (it == m_sections.end())
		{
			throw config_file_exception(
				std::string("Can't remove parameter '" + key + "' because section '" + section + "' not found!")
			);
		}

		// Counter of found parameters
		std::size_t counter = 0;

		// Convert the key to lowercase
		std::string key_lower = utils::string::lower_case(key);

		// Check that the parameter with the specified index exists
		for (std::size_t i = 0; i < it->second.size(); ++i)
		{
			// Сравниваем ключи
			if (it->second[i].data.first.compare(key_lower) == 0)
			{
				// Check the index
				if (index == counter)
				{
					// Remove the parameter
					it->second.erase(it->second.begin() + i);

					// Check that there are parameters in the section.
					if (it->second.empty() == true)
					{
						// Remove the section
						m_sections.erase(it);
					}

					return;
				}

				// Increase the counter
				++counter;
			}
		}

		// The parameter with the specified index was not found
		throw config_file_exception(
			std::string("Can't remove parameter '" + key + "' because index '" + std::to_string(index) + "' not found!")
		);
	}
}