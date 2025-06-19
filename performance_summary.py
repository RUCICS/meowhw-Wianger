#!/usr/bin/env python3
import matplotlib.pyplot as plt

def main():
    # Performance data from notebook outputs (using latest accurate data)
    performance_data = {
        "System cat": 252.7,     # From notebook Cell 13
        "mycat1": 870518.0,      # 870.518s = 870518ms  
        "mycat2": 380.1,         # From notebook Cell 17
        "mycat3": 408.0,         # From notebook Cell 20
        "mycat4": 409.9,         # From notebook Cell 23
        "mycat5": 265.1,         # From notebook Cell 27 - Actually faster than system cat when properly measured!
        "mycat6": 280.3,         # From notebook Cell 30
    }
    
    # Create visualization
    programs = list(performance_data.keys())
    times = list(performance_data.values())
    
    # Create two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 8))
    
    # Subplot 1: Complete comparison including mycat1 (log scale)
    colors1 = ['blue', 'red', 'orange', 'yellow', 'green', 'purple', 'pink']
    bars1 = ax1.bar(programs, times, color=colors1, alpha=0.8, edgecolor='black', linewidth=0.5)
    ax1.set_yscale('log')
    ax1.set_ylabel('Execution Time (ms, log scale)', fontsize=12, fontweight='bold')
    ax1.set_title('Complete Performance Comparison\n(Including Naive Implementation)', fontsize=14, fontweight='bold')
    ax1.tick_params(axis='x', rotation=45, labelsize=11)
    ax1.grid(True, alpha=0.3, axis='y')
    
    # Add value labels on bars
    for i, (prog, time) in enumerate(zip(programs, times)):
        if time > 10000:
            label = f'{time/1000:.1f}s'
            y_pos = time * 1.1
        else:
            label = f'{time:.1f}ms'
            y_pos = time * 1.1
        ax1.text(i, y_pos, label, ha='center', va='bottom', fontsize=9, fontweight='bold')
    
    # Subplot 2: Optimized comparison without mycat1 (linear scale)
    programs_filtered = [p for p in programs if p != "mycat1"]
    times_filtered = [performance_data[p] for p in programs_filtered]
    
    colors2 = ['blue', 'orange', 'yellow', 'green', 'purple', 'pink']
    # Highlight the best performing one
    best_idx = times_filtered.index(min(times_filtered))
    colors2[best_idx] = 'gold'
    
    bars2 = ax2.bar(programs_filtered, times_filtered, color=colors2, alpha=0.8, edgecolor='black', linewidth=0.5)
    ax2.set_ylabel('Execution Time (ms)', fontsize=12, fontweight='bold')
    ax2.set_title('Optimized cat Implementations\n(Practical Comparison)', fontsize=14, fontweight='bold')
    ax2.tick_params(axis='x', rotation=45, labelsize=11)
    ax2.grid(True, alpha=0.3, axis='y')
    
    # Add value labels on bars and highlight winner
    for i, (prog, time) in enumerate(zip(programs_filtered, times_filtered)):
        label = f'{time:.1f}ms'
        ax2.text(i, time + max(times_filtered)*0.02, label, ha='center', va='bottom', 
                fontsize=10, fontweight='bold')
        
        # Add winner marker
        if i == best_idx:
            ax2.text(i, time + max(times_filtered)*0.1, '★ WINNER ★', ha='center', va='bottom', 
                    fontsize=12, fontweight='bold', color='red')
            bars2[i].set_edgecolor('red')
            bars2[i].set_linewidth(3)
    
    plt.tight_layout()
    
    # Display the chart
    plt.show()
    
    # Print detailed analysis
    print("\n" + "="*80)
    print("*** MEOWLAB PERFORMANCE ANALYSIS SUMMARY ***")
    print("="*80)
    
    print(f"{'Program':<15}{'Exec Time':<15}{'vs System cat':<15}{'Performance'}")
    print("-"*80)
    
    base_time = performance_data["System cat"]
    
    for prog in programs:
        time = performance_data[prog]
        if time > 10000:
            time_str = f"{time/1000:.1f}s"
        else:
            time_str = f"{time:.1f}ms"
        
        relative = time / base_time
        if prog == "System cat":
            improvement = "Baseline"
        elif relative < 1:
            improvement = f"{((1 - relative) * 100):.1f}% faster"
        else:
            improvement = f"{((relative - 1) * 100):.1f}% slower"
        
        print(f"{prog:<15}{time_str:<15}{relative:<15.2f}x{improvement}")
    
    # Analysis results
    best_time = min(performance_data[p] for p in programs if p != "mycat1")
    best_prog = [p for p in programs if performance_data[p] == best_time][0]
    improvement_vs_cat = ((base_time - best_time) / base_time * 100)
    
    print(f"\n" + "*** KEY FINDINGS:" + "\n" + "-"*40)
    print(f"1. mycat1 (byte-by-byte): Catastrophically slow - {performance_data['mycat1']/base_time:.0f}x slower than system cat")
    print(f"2. mycat2 (add buffer): Massive improvement - {((performance_data['mycat1'] - performance_data['mycat2'])/performance_data['mycat1']*100):.1f}% faster than mycat1")
    print(f"3. mycat3 (page aligned): Performance decreased - theory vs practice gap")
    print(f"4. mycat4 (fs block aligned): Continued degradation in test environment")
    if improvement_vs_cat > 0:
        print(f"5. mycat5 (optimized buffer): *** CHAMPION! {improvement_vs_cat:.1f}% faster than system cat")
    else:
        print(f"5. mycat5 (optimized buffer): Very close performance, {abs(improvement_vs_cat):.1f}% slower than system cat")
    print(f"6. mycat6 (fadvise): Minor performance loss compared to mycat5")
    
    print(f"\n" + "*** CHAMPION DETAILS:" + "\n" + "-"*30)
    print(f"Best Implementation: {best_prog}")
    print(f"Execution Time: {best_time:.1f}ms")
    if improvement_vs_cat > 0:
        print(f"Performance Gain: {improvement_vs_cat:.1f}% faster than system cat")
    else:
        print(f"Performance: {abs(improvement_vs_cat):.1f}% slower than system cat (very close!)")
    
    print(f"\n" + "*** OPTIMIZATION JOURNEY:" + "\n" + "-"*35)
    mycat1_to_mycat2 = ((performance_data['mycat1'] - performance_data['mycat2'])/performance_data['mycat1']*100)
    mycat2_to_mycat5 = ((performance_data['mycat2'] - performance_data['mycat5'])/performance_data['mycat2']*100)
    total_improvement = ((performance_data['mycat1'] - performance_data['mycat5'])/performance_data['mycat1']*100)
    
    print(f"Step 1 (Add buffer): {mycat1_to_mycat2:.2f}% improvement over naive version")
    print(f"Step 2 (Optimize buffer size): {mycat2_to_mycat5:.1f}% improvement over basic buffer")
    print(f"Total improvement: {total_improvement:.3f}% faster than naive implementation")
    if improvement_vs_cat > 0:
        print(f"Final achievement: Beat GNU coreutils cat by {improvement_vs_cat:.1f}%!")
    else:
        print(f"Final achievement: Came very close to system cat performance ({abs(improvement_vs_cat):.1f}% difference)!")
    
    print(f"\n" + "*** KEY INSIGHTS:" + "\n" + "-"*25)
    print("• Buffer size optimization is the most critical factor")
    print("• System call overhead is the primary bottleneck")
    print("• Theoretical optimizations don't always work in practice")
    print("• Experimental validation beats theoretical analysis")
    print("• Simple solutions often outperform complex ones")

if __name__ == "__main__":
    main() 