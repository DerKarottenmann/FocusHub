#include <cstdint>
#include <cmath>
#include <iostream>

int ledsOnAt(int n, double totalMinutes, double elapsedMinutes) {
    if (n <= 0 || totalMinutes <= 0) return 0;
    if (elapsedMinutes <= 0) return 0;
    if (elapsedMinutes >= totalMinutes) return n;

    double fraction = elapsedMinutes / totalMinutes;
    int leds = static_cast<int>(std::round(fraction * n));

    if (leds < 0) leds = 0;
    if (leds > n) leds = n;

    return leds;
}

int main() {
    int on = ledsOnAt(20, 30.0, 10.0);
    std::cout << "LEDs on: " << on << '\n';
    return 0;
}
