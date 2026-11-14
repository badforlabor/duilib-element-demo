# Element UI 组件 × duilib 支持度对照表

基准：Element UI 2.x（element.eleme.cn）视觉/交互，duilib 仓库 `duilib/duilib@85658d4`（2012 版）实际控件。

状态定义：

| 标记 | 含义 |
| --- | --- |
| ✅ 已有 | duilib 有原生控件，可直接实现演示（允许使用 `borderround`、状态色等皮肤能力） |
| ◐ 部分支持 | 有相近能力或可组合近似，但形态/交互与 Element 有差距 |
| ❌ 需二次开发 | 无对应控件，需新写控件或交互逻辑 |

统计：**55 个视觉组件中，✅ 29 个、◐ 9 个、❌ 17 个**（另“全局配置”为非视觉配置项，不计入）。
“演示”列中的页面均已在 DuiLibElement 中实现。

## 基础

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| Layout 布局 | ◐ 部分支持 | HorizontalLayout / VerticalLayout / TileLayout / ChildLayout；无 24 栅格与响应式断点 | 布局 |
| Container 容器 | ✅ 已有 | Container、三种 Layout、Control；`inset`/`padding`/`childpadding` | 容器 |
| Color 色彩 | ✅ 已有 | 非控件能力：`bkcolor` / `bordercolor` / `textcolor` 样式体系 | 色彩 |
| Typography 字体 | ✅ 已有 | Label / Text + `<Font>` 资源（字号/粗体/颜色） | 字体 |
| Icon 图标 | ◐ 部分支持 | Label 指定符号字体（如 Segoe MDL2 Assets）或图片；无内置图标集 | 图标 |
| Button 按钮 | ✅ 已有 | Button；`borderround` 圆角 + `hotbkcolor`/`pushedtextcolor` 等状态 | 按钮 |
| Link 文字链接 | ✅ 已有 | ELink（Button 派生）：主色文字、悬停变色/下划线、手型光标、禁用置灰；`type` 支持五种状态色 | 文字链接 |

## 表单

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| Radio 单选框 | ✅ 已有 | Option + `group` 分组互斥 | 单选框 |
| Checkbox 多选框 | ✅ 已有 | CheckBox（继承 Option，`selected` 状态） | 多选框 |
| Input 输入框 | ✅ 已有 | Edit（原生 EDIT 子窗口）；placeholder/前后缀/清空需扩展 | 输入框 |
| InputNumber 计数器 | ✅ 已有 | EInputNumber（Container 派生）：Edit 数字输入 + 两侧步进区，`min`/`max`/`step` | 计数器 |
| Select 选择器 | ✅ 已有 | ESelect（Container 派生）：单选/多选标签/可清空/可搜索/远程/分组/创建条目/三档尺寸，浮层自绘选项面板 | 选择器 |
| Cascader 级联选择器 | ✅ 已有 | ECascader：浮层多列联动面板，支持禁用/清空/仅末级/多选/任意级/懒加载/搜索/自定义节点 | 级联选择器 |
| Switch 开关 | ✅ 已有 | ESwitch（Button 派生）：自绘轨道/滑块，点击切换并抛出 valuechanged | 开关 |
| Slider 滑块 | ✅ 已有 | Slider（拖动逻辑内置）；轨道/滑块需图片或自绘皮肤 | 滑块 |
| TimePicker 时间选择器 | ◐ 部分支持 | DateTime 设置时间格式即可选择时间，弹出面板为系统样式 | 日期时间 |
| DatePicker 日期选择器 | ◐ 部分支持 | DateTime 原生日期下拉（系统日历面板，样式不可定制） | 日期时间 |
| DateTimePicker 日期时间选择器 | ◐ 部分支持 | DateTime 使用“日期+时间”格式 | 日期时间 |
| Upload 上传（不复刻） | ❌ 需二次开发 | 无上传控件，需按钮 + 文件对话框 + 列表 | — |
| Rate 评分 | ✅ 已有 | ERate：五角星自绘，悬停预览、半星（`allowhalf`） | 评分 |
| ColorPicker 颜色选择器（不复刻） | ❌ 需二次开发 | 需取色面板/色板控件 | — |
| Transfer 穿梭框（不复刻） | ❌ 需二次开发 | 需双列表 + 移动逻辑 | — |
| Form 表单（不复刻） | ❌ 需二次开发 | 容器可拼布局；校验、联动、错误提示需自建 | — |

## 数据展示

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| Table 表格 | ✅ 已有 | List + ListHeader/ListHeaderItem + ListTextElement；列宽、表头、隔行色、行线、悬停/选中 | 表格 |
| Tag 标签 | ◐ 部分支持 | Label + `borderround`/`bkcolor` 可做静态标签；关闭按钮、动态增删需扩展 | 标签 |
| Progress 进度条 | ✅ 已有 | Progress（值/范围内置）；前景色与圆角需皮肤定制 | 进度条 |
| Tree 树形控件 | ✅ 已有 | ETree（自绘）：展开/折叠、勾选联动（含半选）、懒加载、节点过滤、手风琴、拖拽排序、自定义行尾按钮 | 树形控件 |
| Pagination 分页 | ✅ 已有 | EPagination（自绘）：页码组 + 首尾省略号 + 上/下一页，点击发送 valuechanged | 分页 |
| Badge 标记 | ✅ 已有 | EBadge（Container 派生）：角标水平内缩 10px 压住内容右上角、垂直居中于上边缘（对应 Element 的 right:10px + translateX(100%)），控件尺寸按“内容+角标”AABB 参与布局；数值上限 99+、自定义文本（new/hot）、dot 圆点、五种类型色 | 标记 |
| Avatar 头像 | ✅ 已有 | EAvatar（GDI+）：圆形/方形裁剪，图片、文字、图标三种内容 | 头像 |

## 通知

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| Alert 警告（P2) | ❌ 需二次开发 | Container + Label 可近似；图标、关闭按钮、类型色需自绘 | — |
| Loading 加载 | ✅ 已有 | ELoading（自绘）：抗锯齿旋转弧线，定时器驱动；尺寸/禁用态可配 | 加载 |
| Message 消息提示（P2)  | ❌ 需二次开发 | 需顶部浮层队列 | — |
| MessageBox 弹框（P2)  | ❌ 需二次开发 | 需独立窗口或浮层面板 + 按钮区 | — |
| Notification 通知（P2)  | ❌ 需二次开发 | 需右上角浮层队列 | — |

## 导航

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| NavMenu 导航菜单（P2)  | ❌ 需二次开发 | List/Tree 可做静态菜单，手风琴折叠与选中联动需自建 | — |
| Tabs 标签页 | ✅ 已有 | TabLayout 切换页面 + Button 做标签头 | 标签页 |
| Breadcrumb 面包屑 | ✅ 已有 | EBreadcrumb（HBox 派生）：路径 + "/" 分隔，点击节点切换当前层级 | 面包屑 |
| PageHeader 页头（P2)  | ❌ 需二次开发 | 需标题/返回/操作区组合 | — |
| Dropdown 下拉菜单（P2)  | ❌ 需二次开发 | Combo 仅做值选择；菜单指令、弹出定位需自建 | — |
| Steps 步骤条 | ✅ 已有 | ESteps（自绘）：节点圆环/勾选 + 连接线，`steps`/`active` 控制内容与进度 | 步骤条 |

## 其他

| 组件 | duilib 支持度 | duilib 对应控件 / 实现方式 | 演示 |
| --- | --- | --- | --- |
| Dialog 对话框（P2)  | ❌ 需二次开发 | 需基于 CWindowWnd 或浮层容器自建 | — |
| Tooltip 文字提示 | ◐ 部分支持 | `CControlUI::SetToolTip` 原生 Windows tooltip；深色气泡需自绘 | 文字提示 |
| Popover 弹出框 | ✅ 已有 | EPopover（浮层容器）：气泡 + 箭头自绘，按按钮定位，点击外部关闭 | 弹出框 |
| Popconfirm 气泡确认框（P2)  | ❌ 需二次开发 | 需 Popover + 按钮区 | — |
| Card 卡片（P2)  | ◐ 部分支持 | Container + `borderround`/边框可做静态卡片；阴影与头部插槽需自绘 | — |
| Carousel 走马灯（P2)  | ❌ 需二次开发 | 需分页指示与切换动画 | — |
| Collapse 折叠面板 | ✅ 已有 | ECollapse（VBox 派生）：标题栏 + 内容区，点击展开/收起并重算高度 | 折叠面板 |
| Timeline 时间线（P2)  | ❌ 需二次开发 | 需节点/轴线绘制 | — |
| Divider 分割线 | ✅ 已有 | Control 设 1px 高/宽 + `bkcolor`（带文字分割线用 HBox 组合） | 分割线 |
| Calendar 日历 | ◐ 部分支持 | DateTime 的原生日历弹窗；独立常驻日历需自建 | 日期时间 |
| Image 图片 | ✅ 已有 | Control `bkimage`（GIF 可用 GifAnim）；需图片资源 | 图片 |
| Backtop 回到顶部（P2)  | ❌ 需二次开发 | 需滚动监听 + 悬浮按钮 | — |
| InfiniteScroll 无限滚动 | ✅ 已有 | EInfiniteList（List 派生）：滚动到底发送 `loadmore`，页面追加数据 | 无限滚动 |
| Drawer 抽屉（P2)  | ❌ 需二次开发 | 需侧滑面板 + 遮罩 | — |

## 实现说明

- “✅/◐”页面均使用 duilib 原生控件；圆角统一用 `borderround`（原生圆角裁剪），禁用/悬停色通过控件状态属性或轻量皮肤子类实现。
- 表中“需二次开发”的组件未实现 demo；DuiLibElement 的侧边栏/内容卡片属于应用框架样式，不代表 NavMenu/Card 组件已支持。
- 原生 DateTime 使用系统 SysDateTimePick32，弹出面板样式跟随操作系统，与 Element 外观存在差异，因此标为“部分支持”。
- Tooltip 使用 duilib 内置的 `tooltip` 属性（系统 tooltip），无 Element 深色气泡样式。
- “展示主题demo”复刻 Element 主题预览页的全部示例区块（Color、Typography、Button、Radio…Avatar），使用单个自绘控件按预览页尺寸等比排版；右侧重置/下载、页脚链接等非组件内容不复刻。Transfer 区块保留占位并红字标注 TODO。
- 主题预览区块均可交互：按钮三态悬停/按下、单选/复选切换、Input 键入、InputNumber 加减、Select 与 Cascader 浮层选择、Switch 开关、Slider 拖动、DatePicker 日历翻月与选日、Rate 评分、Tag 关闭、Pagination 翻页、Tree 三级树展开/收起与选中、Menu 横向下拉与嵌套浮层、纵向分组树（展开后页高自适应）、MessageBox 图标+文字内容、Tabs 切换、Collapse 展开收起、Carousel 指示器切换；Badge 按 Element 的锚点（按钮右上角、骑跨顶边）绘制。
