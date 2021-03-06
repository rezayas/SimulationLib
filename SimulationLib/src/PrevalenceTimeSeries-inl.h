#include "../include/SimulationLib/PrevalenceTimeSeries.h"

#include <cassert>

namespace SimulationLib {
    using namespace std;

    template <typename T>
    PrevalenceTimeSeries<T>::PrevalenceTimeSeries
      (string _name, double _timeMax, double _periodLength,
       int _recordPeriod, TimeStatistic *_stats) {

        name              = _name;

        timeMax           = _timeMax;
        periodLength      = _periodLength;
        numPeriods        = (int)(timeMax / periodLength) + 1;

        currentPrevalence = (T)0;

        lastTime          = (T)0;
        lastPeriod        = 0;

        prevalence        = make_shared<vector<T>>(numPeriods, (T)0);

        recordPeriod      = _recordPeriod;
        stats             = _stats;

        writable          = true;
    }

    template <typename T>
    bool PrevalenceTimeSeries<T>::Record(double time, T increment) {
        int thisPeriod;

        if (!writable) {
            printf("Error: TimeSeries has already been closed\n");
            return false;
        }

        // Is 'time' a non-negative integer?
        if (time < 0) {
            printf("Error: 'time' must be non-negative\n");
            return false;
        }

        if (time > timeMax) {
            printf("Error: 'time' cannot be greater than 'timeMax'\n");
            return false;
        }

        if (time < lastTime) {
            printf("Error: Successive calls to ::Record must have \
                      monotonically increasing time\n");
            return false;
        }

        // Update current prevalence, note 'increment' can be positive or
        // negative.
        thisPeriod = (int)ceil(time / periodLength);

        if (stats && recordPeriod == RECORD_ON_ALL)
            stats->Record(time, (double)(currentPrevalence + increment));
        else if (stats &&
                 thisPeriod > lastPeriod &&
                 (lastPeriod % recordPeriod) == 0)
            stats->Record(lastPeriod, (double)currentPrevalence);

        if (thisPeriod > lastPeriod)
        {
            //Check if thisPeriod is the period directly following last period
            if (thisPeriod == (lastPeriod + 1))
                _storePrevalence(lastPeriod);
            // Prevalence values are initially zeroed, so if thisPeriod is
            // not the period directly following lastPeriod, the zeroes must
            // be updated appropriately
            else {
                for (int i = lastPeriod;
                     i < thisPeriod;
                     ++i) {
                    _storePrevalence(i);
                }
            }
        }

        currentPrevalence += increment;
        lastTime = time;
        lastPeriod = thisPeriod;

        return true;
    }

    template <typename T>
    void PrevalenceTimeSeries<T>::Close(void) {
        if (stats != nullptr && recordPeriod > 0)
            stats->Record(lastPeriod, (double)currentPrevalence);

        _storePrevalence(lastPeriod);
        writable = false;

        return;
    }

    template <typename T>
    std::shared_ptr<vector<T>>
    PrevalenceTimeSeries<T>::GetVector(void) {
        if (writable)
            printf("Warning: TimeSeries is still writable\n");

        // Make sure vector is current with last data
        _storePrevalence(lastPeriod);

        return prevalence;
    }

    template <typename T>
    double PrevalenceTimeSeries<T>::GetTime0(void) {
        return 0;
    }

    template <typename T>
    double PrevalenceTimeSeries<T>::GetTimeMax(void) {
        return timeMax;
    }

    template <typename T>
    string PrevalenceTimeSeries<T>::GetName(void) {
        return name;
    }

    template <typename T>
    int PrevalenceTimeSeries<T>::GetPeriodLength(void) {
        return periodLength;
    }

    template <typename T>
    bool PrevalenceTimeSeries<T>::IsWritable(void) {
        return writable;
    }

    template <typename T>
    int PrevalenceTimeSeries<T>::GetNPeriods(void) {
        return numPeriods;
    }

    template <typename T>
    T PrevalenceTimeSeries<T>::GetCurrentPrevalence(void) {
        return currentPrevalence;
    }

    template <typename T>
    T PrevalenceTimeSeries<T>::GetTotalAtTime(double t) const {
        int thisPeriod { (int)ceil(t / periodLength) };

        if (t > timeMax)
            return 0;

        if (thisPeriod >= lastPeriod)
            return currentPrevalence;
        else
            return prevalence->at((int)ceil(t / periodLength));
    }

    template <typename T>
    T PrevalenceTimeSeries<T>::operator()(double t) const {
        return GetTotalAtTime(t);
    }

    template <typename T>
    void PrevalenceTimeSeries<T>::_storePrevalence(int period) {
        assert(period < numPeriods);
        (*prevalence)[period] = currentPrevalence;
    }
}
