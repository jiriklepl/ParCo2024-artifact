#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <memory>
#include <map>
#include <string>

#define REGISTER_EXPERIMENT(name) \
	struct register_experiment_##name { \
		register_experiment_##name() { \
			virtual_experiment::register_experiment(#name, []() -> std::unique_ptr<virtual_experiment> { return std::make_unique<experiment>(); }); \
		} \
	} register_experiment_##name

class virtual_experiment {
protected:
	struct virtual_data {
		virtual ~virtual_data() = default;

		virtual void run() = 0;
		virtual void print_results(std::ostream& os) = 0;
	};

public:
	virtual_experiment() = default;
	virtual ~virtual_experiment() = default;

	void run() {
		data->run();
	}

	void print_results(std::ostream& os) {
		data->print_results(os);
	}

	static auto register_experiment(std::string name, std::unique_ptr<virtual_experiment> (*factory)() ) {
		experiments.try_emplace(std::move(name), factory);
	}

	static void print_experiments(std::ostream& os) {
		std::cerr << "Available experiments:" << std::endl;
		for (const auto& [name, _] : experiments) {
			os << "\t" << name << std::endl;
		}
	}

	static auto get_experiment(const std::string &name) {
		return experiments.at(name)();
	}

protected:
	std::unique_ptr<virtual_data> data;

private:
	static inline std::map<std::string, std::unique_ptr<virtual_experiment> (*)()> experiments;
};

#endif // COMMON_HPP