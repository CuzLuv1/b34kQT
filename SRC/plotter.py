# plotter.py
import matplotlib
matplotlib.use('Agg') # 在无GUI环境（例如从C++调用时）中保存图片至关重要
import matplotlib.pyplot as plt
import numpy as np
import os
import traceback
import matplotlib.ticker as ticker # 导入ticker模块

# --- 中文字体和科研绘图配置 ---
def configure_matplotlib_for_chinese():
    """配置 Matplotlib以支持中文显示。"""
    try:
        plt.rcParams['font.sans-serif'] = ['SimHei', 'DejaVu Sans']
        plt.rcParams['axes.unicode_minus'] = False
        print("Python: [plotter.py] Matplotlib 配置为使用 SimHei (或备选) 字体显示中文。")
    except Exception as e:
        print(f"Python: [plotter.py] 配置中文字体失败: {e}。请确保已安装中文字体 (如 SimHei) 并被 Matplotlib 识别。")

configure_matplotlib_for_chinese()

try:
    plt.style.use('seaborn-v0_8-v0_8_paper')
    plt.rcParams.update({
        'font.size': 10,
        'axes.labelsize': 12,
        'axes.titlesize': 14,
        'xtick.labelsize': 10,
        'ytick.labelsize': 10,
        'legend.fontsize': 10,
        'figure.titlesize': 16,
        'lines.linewidth': 1.8,
        'lines.markersize': 4,
        'axes.grid': True,
        'grid.linestyle': '--',
        'grid.alpha': 0.7,
        'grid.color': 'gray',
    })
except IOError:
    print("Python: [plotter.py] 未找到 Seaborn 样式，使用 Matplotlib 默认样式或自定义 rcParams。")
    # 省略了重复的rcParams设置，如果样式加载失败，这些默认值仍然适用
    pass


def plot_position_differences(list_of_pos_data, output_image_path_base, save_as_svg=False):
    """
    根据从C++传递过来的位置数据列表，创建位置坐标与均值的差值图。
    纵坐标刻度步长固定为0.05，Y轴范围根据数据动态调整并增加边距。
    增加了调试信息打印和最小Y轴范围保证。
    """
    if not list_of_pos_data:
        print("Python: [plotter.py] 未收到用于绘图的数据。")
        return False

    try:
        pos_x_abs = np.array([data.get('posX', np.nan) for data in list_of_pos_data])
        pos_y_abs = np.array([data.get('posY', np.nan) for data in list_of_pos_data])
        pos_z_abs = np.array([data.get('posZ', np.nan) for data in list_of_pos_data])

        if len(pos_x_abs) == 0 or np.all(np.isnan(pos_x_abs)):
            print("Python: [plotter.py] 所有 X 坐标数据为 NaN 或为空，无法绘图。")
            return False

        mean_x = np.nanmean(pos_x_abs)
        mean_y = np.nanmean(pos_y_abs)
        mean_z = np.nanmean(pos_z_abs)

        print(f"Python: [plotter.py] 计算得到的均值 - X: {mean_x:.4f}, Y: {mean_y:.4f}, Z: {mean_z:.4f}")

        diff_x = pos_x_abs - mean_x
        diff_y = pos_y_abs - mean_y
        diff_z = pos_z_abs - mean_z

        indices = np.arange(len(list_of_pos_data))

        fig, axs = plt.subplots(3, 1, figsize=(10, 8), sharex=True, constrained_layout=True)
        fig.suptitle('位置坐标与均值差值图', fontsize=plt.rcParams['figure.titlesize'], y=1.02)

        plot_params = [
            {'data': diff_x, 'label': r'$\Delta X$ (, m)', 'color': 'C0', 'marker': 'o'},
            {'data': diff_y, 'label': r'$\Delta Y$ (, m)', 'color': 'C1', 'marker': 's'},
            {'data': diff_z, 'label': r'$\Delta Z$ (, m)', 'color': 'C2', 'marker': '^'}
        ]

        y_tick_step = 0.05 # 定义纵坐标刻度步长

        for i, params in enumerate(plot_params):
            valid_indices = indices[~np.isnan(params['data'])]
            valid_data = params['data'][~np.isnan(params['data'])]

            axs[i].set_ylabel(params['label']) # 先设置标签

            if len(valid_data) == 0:
                axs[i].text(0.5, 0.5, '无有效数据', horizontalalignment='center', verticalalignment='center', transform=axs[i].transAxes)
                axs[i].yaxis.set_major_locator(ticker.MultipleLocator(base=y_tick_step)) # 即使无数据也设置，保持一致
                axs[i].grid(True, linestyle=plt.rcParams['grid.linestyle'], alpha=plt.rcParams['grid.alpha'], color=plt.rcParams['grid.color'])
                print(f"\n--- Debug Info for Subplot {i} ({params['label']}) ---")
                print("No valid data to plot.")
                print("--- End Debug Info ---")
                continue

            axs[i].plot(valid_indices, valid_data, marker=params['marker'], linestyle='-', label=params['label'], color=params['color'], markersize=plt.rcParams['lines.markersize']-1, linewidth=plt.rcParams['lines.linewidth'])
            axs[i].legend(loc='upper right', frameon=True, edgecolor='black', fancybox=False)
            axs[i].axhline(0, color='black', linewidth=0.8, linestyle='--')

            # --- 设置Y轴刻度步长为0.05 ---
            axs[i].yaxis.set_major_locator(ticker.MultipleLocator(base=y_tick_step))

            # --- 动态调整Y轴范围 ---
            data_min = np.min(valid_data)
            data_max = np.max(valid_data)
            data_range = data_max - data_min

            padding_abs: float
            if data_range == 0:
                padding_abs = y_tick_step
            else:
                padding_abs = data_range * 0.1
            
            ylim_min_candidate = data_min - padding_abs
            ylim_max_candidate = data_max + padding_abs

            if data_min >= 0:
                ylim_min_candidate = min(ylim_min_candidate, -0.1 * y_tick_step) 
            if data_max <= 0:
                ylim_max_candidate = max(ylim_max_candidate, 0.1 * y_tick_step)
            
            if ylim_min_candidate == ylim_max_candidate:
                ylim_min_candidate -= y_tick_step / 2
                ylim_max_candidate += y_tick_step / 2

            # --- 新增：确保Y轴至少有一定范围以显示刻度间隔 ---
            current_span = ylim_max_candidate - ylim_min_candidate
            # 至少留出2个y_tick_step的宽度，这样至少能看到3个刻度（例如 -0.05, 0, 0.05）
            min_desired_span = 2.0 * y_tick_step 
            if current_span < min_desired_span:
                center = (ylim_max_candidate + ylim_min_candidate) / 2.0
                ylim_min_candidate = center - min_desired_span / 2.0
                ylim_max_candidate = center + min_desired_span / 2.0
            
            axs[i].set_ylim(ylim_min_candidate, ylim_max_candidate)
            
            axs[i].grid(True, linestyle=plt.rcParams['grid.linestyle'], alpha=plt.rcParams['grid.alpha'], color=plt.rcParams['grid.color'])

            # --- 调试信息打印 ---
            print(f"\n--- Debug Info for Subplot {i} ({params['label']}) ---")
            print(f"Data (min, max, range): ({data_min:.4f}, {data_max:.4f}, {data_range:.4f})")
            print(f"Initial Ylim candidates (after padding & 0-line adjustment): [{ylim_min_candidate:.4f}, {ylim_max_candidate:.4f}] (before min_span enforce)")
            print(f"Min desired span: {min_desired_span:.4f}, Current span before enforce: {current_span:.4f}")
            
            actual_ylims = axs[i].get_ylim() # 获取应用 set_ylim 后的实际范围
            y_ticks = axs[i].get_yticks()   # 获取 Matplotlib 根据 locator 和范围生成的刻度
            
            print(f"Final Y-limits from get_ylim(): [{actual_ylims[0]:.4f}, {actual_ylims[1]:.4f}]")
            print(f"Generated Y-ticks by Matplotlib: {np.round(y_ticks, decimals=4)}") # 使用decimals关键字参数
            
            if len(y_ticks) > 1:
                tick_diffs = np.diff(y_ticks)
                print(f"Differences between Y-ticks: {np.round(tick_diffs, decimals=4)}")
            else:
                print("Not enough Y-ticks generated to calculate differences (check Y-limits and data range).")
            print("--- End Debug Info ---")

        axs[2].set_xlabel('采样点索引', fontsize=plt.rcParams['axes.labelsize'])

        file_format = 'svg' if save_as_svg else 'png'
        final_image_path = f"{output_image_path_base}.{file_format}"

        if save_as_svg:
            plt.savefig(final_image_path, format='svg', bbox_inches='tight')
            print(f"Python: [plotter.py] 图形已成功保存为 SVG 至 {final_image_path}")
        else:
            plt.savefig(final_image_path, format='png', dpi=300, bbox_inches='tight')
            print(f"Python: [plotter.py] 图形已成功保存为 PNG (300 DPI) 至 {final_image_path}")

        plt.close(fig)
        return True

    except Exception as e:
        print(f"Python: [plotter.py] 绘图过程中发生错误: {e}")
        traceback.print_exc()
        return False

if __name__ == '__main__':
    print("Python: [plotter.py] 以测试模式运行。")

    test_data = []
    for k in range(150): # 修改变量名以避免与内层循环的i冲突
        # 生成一些 posY 数据，使其差值（相对于均值）的波动范围较小，以测试小范围下的刻度情况
        # 假设 posY 的均值是 200
        # 让 diff_y 的范围在 -0.08 到 +0.08 之间
        if k % 3 == 0: # 产生一些更小范围的数据
            raw_y_diff = (np.random.rand() - 0.5) * 0.06 # 范围 (-0.03, 0.03)
        elif k % 7 == 0: # 产生一些精确在0.05倍数上的数据
            raw_y_diff = np.random.choice([-0.05, 0, 0.05, 0.1, -0.1])
        else: # 一般情况
            raw_y_diff = (np.random.rand() - 0.5) * 0.16 # 范围 (-0.08, 0.08)

        pos_y_val = 200.0 + raw_y_diff

        if k % 20 == 0:
             test_data.append({'posX': np.nan, 'posY': pos_y_val , 'posZ': 50.0 + np.random.randn()*0.2})
        elif k % 30 == 0:
             test_data.append({'posX': 100.0 + np.random.randn()*0.1, 'posY': np.nan, 'posZ': 50.0 + np.random.randn()*0.2})
        else:
            test_data.append({'posX': 100.0 + np.random.randn()*0.1,
                              'posY': pos_y_val,
                              'posZ': 50.0 + np.random.randn()*0.2})


    script_dir = os.path.dirname(os.path.abspath(__file__))
    if not script_dir: script_dir = "."
    test_output_dir = os.path.join(script_dir, "test_plots_output_final_cn_debug") # 新的输出目录
    os.makedirs(test_output_dir, exist_ok=True)

    base_output_path = os.path.join(test_output_dir, "test_plot_debug_cn")

    print(f"\nPython: [plotter.py] 尝试将测试图保存为 PNG...")
    plot_position_differences(test_data, base_output_path, save_as_svg=False)

    print(f"\nPython: [plotter.py] 尝试将测试图保存为 SVG...")
    plot_position_differences(test_data, base_output_path, save_as_svg=True) # 保存SVG以供检查
    print("Python: [plotter.py] 测试完成。")