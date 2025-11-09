# O.R.C.H.I.D. — Outrageously Repurposed Calculator Human Interface Device


![Picture of a pink orchid flower on a white background. Photo by <a href="https://unsplash.com/@zoltantasi?utm_source=unsplash&utm_medium=referral&utm_content=creditCopyText">Zoltan Tasi</a> on <a href="https://unsplash.com/photos/purple-moth-orchids-in-bloom-PN8Un1ywbE8?utm_source=unsplash&utm_medium=referral&utm_content=creditCopyText">Unsplash</a>](images/zoltan-tasi-PN8Un1ywbE8-unsplash.jpg)

Many are familiar with the awesome abilities of the TI-84 calculator to run custom programs for simulation, math and science, and even games. However, most of these programs are written just to be run on the calculator itself. Many are unaware that the TI-84 CE calculator includes a USB port that supports interfacing with external devices using HID. This means the TI-84 calculator can simulate computer mice, keyboards, and even [magic carpets](https://usb.org/sites/default/files/hut1_6.pdf#page=71).

This program can be loaded on to the TI-84 Plus CE graphing calculator to make the calculator act as an external mouse and keyboard by implementing the USB HID protocol.

Contributors: Peter Lilley and Eric Butcher

## How We Built It

This program was written in C  and compiled using the [CE C/C++ Toolchain](https://ce-programming.github.io/toolchain/index.html). This allowed us to use USB libraries to handle communication and focus on creating the HID communication between the host and calculator. 

## Challenges We Ran Into

The USB HID protocol ended up being much more complicated than we initially thought it was. The HID protocol itself and all the open-source repositories implementing HID we found as inspiration were filled with large byte arrays of magic hex values which were very difficult to decipher. After hour reading cryptic documentation, we figured out how the protocol works and were successfully able to create HID report descriptors to configure mouse and keyboard input from the calculator to the host. 

## Accomplishments We Are Proud Of

Learning the HID protocol from scratch and getting a calculator to work as a USB input device. 

## What We Learned

We learned the HID protocol. 

