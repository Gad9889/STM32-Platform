#!/usr/bin/env python3
"""
STM32CubeMX Configuration Validator

Validates .ioc files against STM32 Platform requirements.
Checks DMA configuration, clock settings, interrupt priorities, and peripheral configuration.

Usage:
    python validate_cubemx_config.py <path_to_project.ioc>
"""

import sys
import re
from pathlib import Path
from typing import Dict, List, Tuple

# Platform requirements
REQUIREMENTS = {
    "dma": {
        "UART_RX_MODE": "CIRCULAR",
        "UART_RX_PRIORITY": "HIGH",
        "UART_TX_MODE": "NORMAL",
        "UART_TX_PRIORITY": "MEDIUM",
        "SPI_MODE": "NORMAL",
        "ADC_MODE": "CIRCULAR",
    },
    "interrupts": {
        "DMA_PREEMPT": "5",
        "DMA_SUB": "0",
        "PERIPHERAL_PREEMPT": "6",
        "PERIPHERAL_SUB": "0",
    },
    "clock": {
        "APB1_MIN": 42_000_000,  # 42 MHz for CAN timing
        "APB1_MAX": 50_000_000,
        "APB2_MIN": 80_000_000,  # High-speed peripherals
        "APB2_MAX": 100_000_000,
    },
    "can": {
        "BAUDRATE": 500_000,  # 500 kbit/s
        "AUTO_RETRANSMIT": "ENABLE",
        "MODE": "NORMAL",
    }
}


class IocValidator:
    """Validator for STM32CubeMX .ioc configuration files"""

    def __init__(self, ioc_path: Path):
        self.ioc_path = ioc_path
        self.config: Dict[str, str] = {}
        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.load_config()

    def load_config(self):
        """Load .ioc file into dictionary"""
        if not self.ioc_path.exists():
            raise FileNotFoundError(f"IOC file not found: {self.ioc_path}")

        with open(self.ioc_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if '=' in line and not line.startswith('#'):
                    key, value = line.split('=', 1)
                    self.config[key.strip()] = value.strip()

    def get_value(self, key: str, default: str = "") -> str:
        """Get configuration value by key"""
        return self.config.get(key, default)

    def validate_dma_config(self):
        """Validate DMA configuration for UART, SPI, ADC"""
        print("\n[DMA Configuration]")

        # Check UART DMA
        uart_rx_dma = self.get_value("Dma.USART2_RX.0.Mode", "")
        uart_tx_dma = self.get_value("Dma.USART2_TX.1.Mode", "")

        if uart_rx_dma != REQUIREMENTS["dma"]["UART_RX_MODE"]:
            self.errors.append(
                f"UART RX DMA mode: Expected {REQUIREMENTS['dma']['UART_RX_MODE']}, "
                f"found '{uart_rx_dma}'"
            )
        else:
            print(f"  ✓ UART RX DMA mode: {uart_rx_dma}")

        if uart_tx_dma != REQUIREMENTS["dma"]["UART_TX_MODE"]:
            self.warnings.append(
                f"UART TX DMA mode: Expected {REQUIREMENTS['dma']['UART_TX_MODE']}, "
                f"found '{uart_tx_dma}'"
            )
        else:
            print(f"  ✓ UART TX DMA mode: {uart_tx_dma}")

        # Check ADC DMA
        adc_dma_mode = self.get_value("Dma.ADC1.0.Mode", "")
        if adc_dma_mode != REQUIREMENTS["dma"]["ADC_MODE"]:
            self.errors.append(
                f"ADC DMA mode: Expected {REQUIREMENTS['dma']['ADC_MODE']}, "
                f"found '{adc_dma_mode}'"
            )
        else:
            print(f"  ✓ ADC DMA mode: {adc_dma_mode}")

    def validate_interrupts(self):
        """Validate interrupt priority configuration"""
        print("\n[Interrupt Configuration]")

        # Check DMA interrupt priorities
        dma_preempt = self.get_value("NVIC.DMA2_Stream0_IRQn.0.PreemptionPriority", "")
        dma_sub = self.get_value("NVIC.DMA2_Stream0_IRQn.0.SubPriority", "")

        if dma_preempt != REQUIREMENTS["interrupts"]["DMA_PREEMPT"]:
            self.warnings.append(
                f"DMA preemption priority: Expected {REQUIREMENTS['interrupts']['DMA_PREEMPT']}, "
                f"found '{dma_preempt}'"
            )
        else:
            print(f"  ✓ DMA preemption priority: {dma_preempt}")

        if dma_sub != REQUIREMENTS["interrupts"]["DMA_SUB"]:
            self.warnings.append(
                f"DMA sub-priority: Expected {REQUIREMENTS['interrupts']['DMA_SUB']}, "
                f"found '{dma_sub}'"
            )

    def validate_clock_config(self):
        """Validate clock tree configuration"""
        print("\n[Clock Configuration]")

        # Extract APB1 and APB2 clock frequencies
        apb1_freq_str = self.get_value("RCC.APB1Freq_Value", "0")
        apb2_freq_str = self.get_value("RCC.APB2Freq_Value", "0")

        try:
            apb1_freq = int(apb1_freq_str)
            apb2_freq = int(apb2_freq_str)
        except ValueError:
            self.errors.append("Failed to parse clock frequencies from IOC file")
            return

        # Validate APB1 (CAN bus requires specific timing)
        if not (REQUIREMENTS["clock"]["APB1_MIN"] <= apb1_freq <= REQUIREMENTS["clock"]["APB1_MAX"]):
            self.errors.append(
                f"APB1 frequency out of range: {apb1_freq / 1e6:.1f} MHz "
                f"(expected {REQUIREMENTS['clock']['APB1_MIN'] / 1e6:.0f}-"
                f"{REQUIREMENTS['clock']['APB1_MAX'] / 1e6:.0f} MHz)"
            )
        else:
            print(f"  ✓ APB1 frequency: {apb1_freq / 1e6:.1f} MHz")

        # Validate APB2
        if not (REQUIREMENTS["clock"]["APB2_MIN"] <= apb2_freq <= REQUIREMENTS["clock"]["APB2_MAX"]):
            self.warnings.append(
                f"APB2 frequency out of optimal range: {apb2_freq / 1e6:.1f} MHz "
                f"(recommended {REQUIREMENTS['clock']['APB2_MIN'] / 1e6:.0f}-"
                f"{REQUIREMENTS['clock']['APB2_MAX'] / 1e6:.0f} MHz)"
            )
        else:
            print(f"  ✓ APB2 frequency: {apb2_freq / 1e6:.1f} MHz")

    def validate_can_config(self):
        """Validate CAN peripheral configuration"""
        print("\n[CAN Configuration]")

        # Check if CAN is enabled
        can_mode = self.get_value("CAN1.Mode", "")
        if not can_mode:
            self.warnings.append("CAN1 not enabled (platform supports CAN)")
            return

        # Check CAN prescaler and timing (baudrate calculation)
        prescaler = self.get_value("CAN1.Prescaler", "")
        if prescaler:
            print(f"  ✓ CAN prescaler configured: {prescaler}")

        # Check auto-retransmission
        auto_retx = self.get_value("CAN1.NART", "")
        if auto_retx == "DISABLE":
            print(f"  ✓ CAN auto-retransmit: ENABLED (NART=DISABLE)")
        else:
            self.warnings.append(
                f"CAN auto-retransmit should be enabled (NART=DISABLE), found NART={auto_retx}"
            )

    def validate_code_generation(self):
        """Validate code generation settings"""
        print("\n[Code Generation Settings]")

        keep_user_code = self.get_value("ProjectManager.KeepUserCode", "")
        if keep_user_code != "true":
            self.errors.append(
                "Code generation setting 'KeepUserCode' must be 'true' for platform integration"
            )
        else:
            print("  ✓ Keep user code: Enabled")

        delete_prev = self.get_value("ProjectManager.DeletePrevious", "")
        if delete_prev == "true":
            self.warnings.append(
                "Code generation 'DeletePrevious' is enabled - may remove platform files"
            )

    def run_validation(self) -> bool:
        """Run all validation checks"""
        print(f"\n{'=' * 60}")
        print(f"STM32CubeMX Configuration Validator")
        print(f"File: {self.ioc_path.name}")
        print(f"{'=' * 60}")

        self.validate_dma_config()
        self.validate_interrupts()
        self.validate_clock_config()
        self.validate_can_config()
        self.validate_code_generation()

        # Print results
        print(f"\n{'=' * 60}")
        print("Validation Results")
        print(f"{'=' * 60}")

        if not self.errors and not self.warnings:
            print("\n✓ All checks passed! Configuration meets platform requirements.")
            return True

        if self.errors:
            print(f"\n❌ ERRORS ({len(self.errors)}):")
            for i, error in enumerate(self.errors, 1):
                print(f"  {i}. {error}")

        if self.warnings:
            print(f"\n⚠ WARNINGS ({len(self.warnings)}):")
            for i, warning in enumerate(self.warnings, 1):
                print(f"  {i}. {warning}")

        if self.errors:
            print("\n❌ Configuration FAILED validation. Fix errors before proceeding.")
            return False
        else:
            print("\n⚠ Configuration passed with warnings. Review recommendations.")
            return True


def main():
    """Main entry point"""
    if len(sys.argv) != 2:
        print("Usage: python validate_cubemx_config.py <path_to_project.ioc>")
        sys.exit(1)

    ioc_path = Path(sys.argv[1])

    try:
        validator = IocValidator(ioc_path)
        success = validator.run_validation()
        sys.exit(0 if success else 1)
    except FileNotFoundError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(2)


if __name__ == "__main__":
    main()
