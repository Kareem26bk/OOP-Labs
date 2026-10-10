# Object-Oriented Programming (C++) - Universal IR Remote Control

This repository contains progressive laboratory assignments focused on applying Object-Oriented Programming (OOP) principles in C++ for embedded systems. The project builds a universal infrared (IR) remote control utilizing an ESP32 microcontroller (M5StickC)[cite: 33]. The codebase evolves from basic hardware encapsulation to advanced software architecture patterns.

## 🚀 Skills & Concepts Demonstrated
* **Encapsulation & State Management:** Hiding internal hardware states using private members, accessed exclusively via getter and setter methods[cite: 31, 33].
* **Abstraction & Interfaces:** Designing pure virtual methods and abstract base classes to decouple hardware logic from presentation layers[cite: 29].
* **Polymorphism (Dynamic Binding):** Utilizing base class pointers to dynamically resolve and execute overridden methods at runtime[cite: 29, 30].
* **Multiple Inheritance:** Implementing interface classes alongside concrete base classes to enforce structural contracts[cite: 29].
* **RAII (Resource Acquisition Is Initialization):** Managing hardware resources safely through stack-allocated objects, ensuring deterministic behavior via constructors and destructors[cite: 29, 30].

## 📁 Laboratory Breakdown

### Lab 1: Introduction to Object-Oriented Hardware Encapsulation
* Developed a `Button` class that initializes the hardware pin in the constructor and implements an `isPressed()` method[cite: 33].
* Implemented debouncing logic to prevent multiple rapid triggers[cite: 33].
* Created an `IRDevice` class where the constructor accepts specific device information, such as the address and power code[cite: 33].
* Linked the logic in the main loop so that a registered button press triggers the `togglePower()` method to transmit the IR signal[cite: 33].

### Lab 2: State Management and Display Integration
* Expanded the `IRDevice` class with private member variables for device codes, power-on codes, and power-off codes[cite: 31].
* Introduced an internal boolean state variable (`_isOn`) to track the current power status of the device[cite: 31, 32].
* Updated the `togglePower()` method to evaluate the internal state and transmit either the specific power-on or power-off code accordingly[cite: 32].
* Created a dedicated Display class that receives an initialized display object reference in its constructor[cite: 32].
* Implemented display methods to render the active IR codes and the current power status[cite: 32].

### Lab 3: Advanced OOP - The Universal Remote
* Created an abstract base class named `AbstractDevice` containing a pure virtual method `togglePower() = 0;`[cite: 29].
* Developed specific derived classes (`TVDevice` and `ACDevice`) that override the virtual power method[cite: 29].
* Implemented multiple inheritance by creating a pure interface class `IDisplayable` with a `getDisplayName()` method[cite: 29].
* Refactored the Display class to accept only `IDisplayable*` pointers, completely stripping its dependency on specific IR implementations[cite: 29].
* Utilized polymorphism in the main loop by iterating through an array of `AbstractDevice*` pointers[cite: 30].
* Applied the RAII principle by creating a `TransmitLedGuard` helper class allocated on the stack to guarantee the status LED always turns off when the transmission method exits[cite: 30].
