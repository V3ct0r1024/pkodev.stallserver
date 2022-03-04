#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

namespace pkodev
{
	// Define the type for settings parameter
	struct ConfigParam;

	// Define the type for settings parameter list
	typedef std::vector<ConfigParam> param_list_t;

	// Define the type for settings sections list
	typedef std::map<std::string, param_list_t> config_sections_t;


	// Settings file exception class
	class config_file_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			config_file_exception(const char* what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			config_file_exception(const std::string& what) :
				std::runtime_error(what) { }

	};

	// Parameter structure
	struct ConfigParam final
	{
		// Constructor
		ConfigParam();

		// Constructor
		ConfigParam(const std::string& key_, const std::string& value_);

		// Convert the parameter to various data types
		int                to_integer() const;	// Convert the parameter to int
		double             to_double()  const;	// Convert the parameter to double
		bool               to_bool()    const;	// Convert the parameter to bool
		const std::string& to_string()  const;	// Convert the parameter to std::string

		// key=value pair
		std::pair<std::string, std::string> data;
	};

	// Settings file class
	class ConfigFile final
	{
		public:

			// Constructor
			ConfigFile();

			// Copy constructor
			ConfigFile(const ConfigFile&) = delete;

			// Move constructor
			ConfigFile(ConfigFile&&) = delete;

			// Destructor 
			~ConfigFile();

			// Copy assignment operator
			ConfigFile& operator=(const ConfigFile&) = delete;

			// Move assignment operator
			ConfigFile& operator=(ConfigFile&&) = delete;

			// Load settings from file
			void load(const std::string& path);

			// Save settings to file
			void save(const std::string& path);

			// Get the number of parameters in a section with the same key
			std::size_t param_count(const std::string& section, const std::string& key) const;

			// Check that a parameter exists
			inline bool has(const std::string& section, const std::string& key) const { return (param_count(section, key) > 0); }

			// Get a parameter
			const ConfigParam& get(const std::string& section, const std::string& key,
				const std::string& default_value = "", std::size_t index = 0) const;

			// Set a parameter
			std::size_t set(const std::string& section, const std::string& key,
				const std::string& value, std::size_t index = UINT_MAX);

			// Remove a parameter
			void remove(const std::string& section, const std::string& key, std::size_t index = 0);

		private:

			// Sections list
			config_sections_t m_sections;

			// Null object pattern
			mutable ConfigParam m_null;
	};
}

