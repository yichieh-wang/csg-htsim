#include <cstdlib>
#include <climits>
#include <random>

using namespace std;

// htsim's own generator, under its own names: defined as `rand`/`srand`/`random`/`srandom` these
// replaced libc's for every program htsim is linked into, reseeded from the wall clock by RoceSrc.
// htsim's own calls to rand()/random() bind to libc now.
static mt19937 random_engine;

void htsim_srand(unsigned seed)
{
    random_engine = mt19937(seed);
}

int htsim_rand()
{
    return random_engine() & INT_MAX;
}

void htsim_srandom(unsigned seed)
{
    htsim_srand(seed);
}

long htsim_random()
{
    return htsim_rand();
}
