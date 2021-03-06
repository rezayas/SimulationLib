#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <functional>

#include "PyramidData.h"

using namespace std;

namespace SimulationLib
{
	// to store a collection of pyramid data collected during a simulation run
	class PyramidTimeSeries
	{
	private:
		string name;
		int time0;
		int timeMax;
		int lastTime;
		int lastPeriod;
		int periodLength;
		int nCategories;
		int nPeriods;
		bool closed;
		bool reset;

		std::vector<int> currentValues;
		std::vector<double> ageBreaks;
		std::vector< PyramidData<int> > pyramids;

		// This is implemented in PrevalencePyramidTimeSeries and
		//   IncidencePyramidTimeSeries, abiding by the Template Method pattern
		//   described here: http://www.gotw.ca/publications/mill18.htm
		// Since this function can express the entire functional difference
		//   between Prevalence and Incidence, this pattern elegantly allows the
		//   specification of the two derived classes with minimal code.
		virtual int calcThisPeriod(int time, int periodLength) = 0;

		int _getAgeIdx(double age);
		int _getTotalInCurrentValues(int periodIdx, bool categoryBool, int categoryIdx, \
									 bool ageGroupBool, int ageGroupIdx);

		bool _storeCurrentValues(int period);

	public:
		using value_type      = int;
		using query_type      = std::tuple<int, int, int>;
		using query_signature = int(int, int, int);

		// Initializes a new PyramidTimeSeries.
		//
		// 'name': Name of the time series
		//
		// 'time0': Index of the earliest time entered in the series. All times
		//   entered into the PyramidTimeSeries must be >0 'time0'
		//
		// 'timeMax': The maximum time to be entered in the series. Must be
		//   >= 'time0'
		//
		// 'periodLength': The length of each aggregation period. Since times
		//   are currently specified as integers, a 'periodLength' of 1 will
		//   correspond to a period for each legal value of 'time'
		//
		// 'calcNPeriods': A lambda function which, given 'timeMax' and
		//   'periodLength', calculates the number of periods which should be
		//   stored.
		//
		// 'nCategories': The number of categories (e.g. sex, race) to allow
		//   specification of. Must be >= 1.
		//
		// 'ageBreaks': A vector of the points of division of age. An empty vector
		//   will correspond to persons of any age being lumped into the same
		//   bin. A vector of {10, 20} would correspond to three bins with the
		//   following support: [0, 10), [10, 20), [20, +inf]
		//
		// An out-of-range exception will be thrown for invalid specifications
		//   of any of these parameters
		PyramidTimeSeries(string name, int time0, int timeMax, int periodLength,  \
						  function<int (int, int)> calcNPeriods, int nCategories, \
						  vector<double> ageBreaks, bool reset);

		// Given a 'time'     (time0 <= time <= timeMax),
		// 		   'category' (0 <= category < nCategories)
		// 		   'age'	  (age >= 0), updates the data structure by changing
		//   the number of individuals in the specified group by 'increment'.
		//   Note that 'age' is translated into its respective age bin. Throws
		//     exceptions for illegal values, and returns false if 'increment'
		//     would cause the population of the group to fall below zero, or
		//     if the TimeSeries has been Close()'d.
		bool UpdateByAge(int time, int category, double age, int increment);

		// Identical to UpdateByAge, except age is specified using age group index,
		//  rather than an age in years.
		bool UpdateByIdx(int time, int category, int ageGroupIdx, int increment);

		// Moves 'increment' individuals from the specified old group to the
		//   specified new group. Throws exceptions for illegal parameters.
		//   'increment' must be a non-negative integer. Returns false if the
		//   move would cause the number of individuals in the specified old
		//   group to become negative, or if the TimeSeries has been Close()'d.
		bool MoveByAge(int time, int oldCategory, double oldAge, \
						         int newCategory, double newAge, int increment);

		// Identical to MoveByAge, but the old and new groups are specified
		//   by an age group index, rather than an age in years.
		bool MoveByIdx(int time, int oldCategory, int oldAgeGroupIdx, \
						         int newCategory, int newAgeGroupIdx, int increment);

		value_type GetTotalAtPeriod(int periodIdx);
		value_type GetTotalAtTime(int time);

		value_type GetTotalInCategoryAtPeriod(int periodIdx, int category);
		value_type GetTotalInCategoryAtTime(int time, int category);

		value_type GetTotalInAgeGroupAtPeriod(int periodIdx, int ageGroupIdx);
		value_type GetTotalInAgeGroupAtTime(int time, int ageGroupIdx);

		value_type GetTotalInAgeGroupAndCategoryAtPeriod(int periodIdx, int ageGroupIdx, int category);
		value_type GetTotalInAgeGroupAndCategoryAtTime(int time, int ageGroupIdx, int category);
		value_type operator()(int time, int ageGroupIdx, int category);

		void Close(void);

		// Returns false if data is closed, true otherwise
		bool IsWritable(void);

		// Get the number of categories
		int GetNumberCategories(void);

		// Return a copy of the vector of age breaks
		vector<double> GetAgeBreaks(void);

		// Return the length of the period
		int GetPeriodLength(void);

		// Return the max time
		int GetTimeMax(void);

		// Returns the number of periods
		int GetNPeriods(void);

		// Returns a copy of the vector of current values
		vector<int> GetCurrentValues(void);

		// Return the time0
		int GetTime0(void);

		// Return the name of the PyramidTimeSeries
		string GetName(void);


		// For debugging, prints out all of the contents of the Pyramid Time Series
		void PrintPyramidTimeSeries(void);

		void PrintCurrentValues(void);
	};
}
