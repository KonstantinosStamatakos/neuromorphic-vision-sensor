# Neuromorphic Event-Based Vision Sensor

A two-pixel event-based vision sensor that detects changes in illumination and estimates one-dimensional motion direction from the relative timing of asynchronous events.

The system combines a custom photodiode-based analog front end, interrupt-driven ESP32 firmware, and a Python analysis pipeline for experimental characterization and visualization.

<p align="center">
  <img src="images/prototype.jpg" width="700">
</p>

---

## Overview

Conventional cameras acquire complete image frames at fixed intervals, including information from regions that have not changed.

Event-based vision uses a different sensing paradigm: pixels respond asynchronously to changes in illumination rather than continuously producing complete frames.

This project implements a simplified two-pixel event-based sensor to explore this principle in hardware.

Each pixel consists of a photodiode and analog signal-conditioning circuitry. Changes in incident light produce electrical signals that are thresholded into discrete events. These events are captured asynchronously by an ESP32 using hardware interrupts.

By comparing the timestamps of events produced by two spatially separated sensing channels, the system estimates whether an object moved:

- Pixel A → Pixel B
- Pixel B → Pixel A

The goal is not to reconstruct an image, but to demonstrate how useful motion information can be extracted directly from sparse temporal events.

---

## System Architecture

```text
                    Moving Object
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
         Photodiode A          Photodiode B
              │                     │
              ▼                     ▼
        Analog Front End      Analog Front End
              │                     │
              ▼                     ▼
           Event A               Event B
              │                     │
              └──────────┬──────────┘
                         ▼
                       ESP32
                         │
                  GPIO Interrupts
                         │
                  Event Timestamping
                         │
                 Temporal Correlation
                         │
                         ▼
              Motion Direction Estimate
                         │
                         ▼
                    Serial / CSV
                         │
                         ▼
                  Python Analysis
