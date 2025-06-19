#!/usr/bin/env python3
import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np

def test_buffer_size(buffer_size, test_size_mb=2048):
    """Test performance of specified buffer size"""
    try:
        # Use dd to read from /dev/zero and write to /dev/null
        cmd = [
            'dd', 
            'if=/dev/zero', 
            'of=/dev/null', 
            f'bs={buffer_size}', 
            f'count={test_size_mb * 1024 * 1024 // buffer_size}',
            'status=progress'
        ]
        
        start_time = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True)
        end_time = time.time()
        
        duration = end_time - start_time
        
        # Extract speed information from dd output
        lines = result.stderr.strip().split('\n')
        for line in lines:
            if 'bytes/sec' in line or 'MB/s' in line or 'GB/s' in line:
                # Parse speed
                parts = line.split()
                for i, part in enumerate(parts):
                    if 'bytes/sec' in part:
                        speed = float(parts[i-1])
                        return duration, speed / (1024 * 1024)  # MB/s
                    elif 'MB/s' in part:
                        speed = float(parts[i-1])
                        return duration, speed
                    elif 'GB/s' in part:
                        speed = float(parts[i-1])
                        return duration, speed * 1024
        
        # If speed cannot be parsed, calculate estimated value
        speed_mbs = (test_size_mb) / duration
        return duration, speed_mbs
        
    except Exception as e:
        print(f"Error testing buffer size {buffer_size}: {e}")
        return None, None

def main():
    # Base block size (4KB)
    base_size = 4096
    
    # Test different multipliers
    multipliers = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
    
    results = []
    
    print("Testing different buffer sizes performance...")
    print("Multiplier\tBuffer Size\tTime(s)\tSpeed(MB/s)")
    print("-" * 55)
    
    for multiplier in multipliers:
        buffer_size = base_size * multiplier
        duration, speed = test_buffer_size(buffer_size, test_size_mb=512)  # Use smaller test size
        
        if duration is not None and speed is not None:
            results.append((multiplier, buffer_size, duration, speed))
            print(f"{multiplier}\t\t{buffer_size}\t\t{duration:.3f}\t{speed:.1f}")
        else:
            print(f"{multiplier}\t\t{buffer_size}\t\tFailed\tFailed")
    
    # Create plots - using English titles to avoid font issues
    if results:
        multipliers_plot = [r[0] for r in results]
        speeds = [r[3] for r in results]
        
        plt.figure(figsize=(14, 6))
        
        # 子图1：速度对比线图
        plt.subplot(1, 2, 1)
        plt.plot(multipliers_plot, speeds, 'bo-', linewidth=2, markersize=8)
        plt.xlabel('Buffer Size Multiplier', fontsize=12)
        plt.ylabel('Speed (MB/s)', fontsize=12)
        plt.title('Read/Write Speed vs Buffer Size', fontsize=14, fontweight='bold')
        plt.grid(True, alpha=0.3)
        plt.xscale('log', base=2)
        
        # 添加最优点标注
        max_speed = max(speeds)
        optimal_idx = speeds.index(max_speed)
        optimal_multiplier = multipliers_plot[optimal_idx]
        plt.annotate(f'Optimal: {optimal_multiplier}x\n{max_speed:.0f} MB/s', 
                    xy=(optimal_multiplier, max_speed), 
                    xytext=(optimal_multiplier*2, max_speed*0.8),
                    arrowprops=dict(arrowstyle='->', color='red'),
                    fontsize=10, ha='center',
                    bbox=dict(boxstyle="round,pad=0.3", facecolor="yellow", alpha=0.7))
        
        # 子图2：柱状图
        plt.subplot(1, 2, 2)
        colors = ['skyblue'] * len(multipliers_plot)
        colors[optimal_idx] = 'gold'  # Highlight best performance
        
        bars = plt.bar(range(len(multipliers_plot)), speeds, color=colors, alpha=0.8, edgecolor='navy', linewidth=0.5)
        plt.xlabel('Buffer Size Multiplier', fontsize=12)
        plt.ylabel('Speed (MB/s)', fontsize=12)
        plt.title('Buffer Size Performance Comparison', fontsize=14, fontweight='bold')
        plt.xticks(range(len(multipliers_plot)), [str(m) for m in multipliers_plot], rotation=45)
        plt.grid(True, axis='y', alpha=0.3)
        
        # Add label on the highest bar
        bars[optimal_idx].set_edgecolor('red')
        bars[optimal_idx].set_linewidth(2)
        plt.text(optimal_idx, max_speed + max_speed*0.01, f'{max_speed:.0f}', 
                ha='center', va='bottom', fontweight='bold', fontsize=10)
        
        plt.tight_layout()
        
        # Display in notebook
        plt.show()
        
        # Analyze results
        print(f"\n" + "="*60)
        print(f"BUFFER SIZE OPTIMIZATION ANALYSIS")
        print(f"="*60)
        print(f"Max Speed Achieved: {max_speed:.1f} MB/s")
        print(f"Optimal Multiplier: {optimal_multiplier}x")
        print(f"Optimal Buffer Size: {base_size * optimal_multiplier:,} bytes ({base_size * optimal_multiplier // 1024} KB)")
        
        # Find performance balance point
        # Look for 90% performance point as balance point
        threshold_speed = max_speed * 0.9
        good_multipliers = [m for m, s in zip(multipliers_plot, speeds) if s >= threshold_speed]
        recommended_multiplier = min(good_multipliers) if good_multipliers else optimal_multiplier
        
        print(f"Recommended Multiplier: {recommended_multiplier}x (90% performance threshold)")
        print(f"Threshold Speed: {threshold_speed:.1f} MB/s")
        
        # Performance improvement analysis
        baseline_speed = speeds[0]  # Speed of 1x multiplier
        improvement = ((max_speed - baseline_speed) / baseline_speed) * 100
        print(f"Performance Improvement: {improvement:.1f}% over 1x buffer")
        
        return recommended_multiplier
    
    return 1

if __name__ == "__main__":
    main() 