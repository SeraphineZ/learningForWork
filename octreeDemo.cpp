#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCubeSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCamera.h>
#include <vector>
#include <random>

// 点云结构
struct Point3D
{
    double x, y, z;

    /**
     * @brief 构造一个三维点对象。
     *
     * 使用给定的x、y、z坐标初始化一个三维点对象。默认情况下，
     * 所有坐标被初始化为0。
     *
     * @param _x 点的x坐标。
     * @param _y 点的y坐标。
     * @param _z 点的z坐标。
     */
    Point3D(double _x = 0, double _y = 0, double _z = 0)
        : x(_x), y(_y), z(_z) {}
};

class OctreeNode
{
public:
    Point3D center;
    double size;
    int depth;
    std::vector<Point3D> points;
    std::vector<OctreeNode *> children;

    /**
     * @brief 构造一个具有指定中心、大小和深度的八叉树节点。
     *
     * 初始化八叉树节点的中心点、所代表的立方体空间的大小以及在八叉树中的深度级别。默认情况下，深度设置为0。
     *
     * @param _center 八叉树节点的中心点。
     * @param _size 节点所代表的立方体空间的大小。
     * @param _depth 节点在八叉树中的深度级别（默认为0）。
     */
    OctreeNode(const Point3D &_center, double _size, int _depth = 0)
        : center(_center), size(_size), depth(_depth) {}

    /**
     * @brief 判断该节点是否是叶子节点。
     *        叶子节点是没有子节点的节点。
     * @return 如果该节点是叶子节点，返回 true，否则返回 false。
     */
    bool isLeaf() const
    {
        return children.empty();
    }
};

class Octree
{
private:
    const int MAX_DEPTH;
    const int MIN_POINTS;
    OctreeNode *root;

    /**
     * @brief 根据点的坐标和八叉树中心点，
     *        计算该点在八叉树中的八面体索引（octant）。
     *
     * 该函数将点的坐标与八叉树中心点的坐标进行
     * 比较，计算出该点在八叉树中的八面体索引（
     * octant）。八面体索引使用三个位来表示x、y、z
     * 三个轴上的位置（0表示负方向，1表示正方向）。
     *
     * @param point 需要计算的点的坐标。
     * @param center 八叉树中心点的坐标。
     * @return 该点在八叉树中的八面体索引（octant）。
     */
    int getOctant(const Point3D &point, const Point3D &center)
    {
        int octant = 0;
        if (point.x >= center.x)
            octant |= 4;
        if (point.y >= center.y)
            octant |= 2;
        if (point.z >= center.z)
            octant |= 1;
        return octant;
    }

    /**
     * @brief 计算给定八叉体子节点的中心点。
     *
     * 根据父节点的中心点、指定的子空间索引（octant）和子节点的大小，
     * 计算并返回该子节点的中心点。子空间索引使用三个位来表示x、y、z
     * 三个轴上的位置（0表示负方向，1表示正方向）。
     *
     * @param parent_center 父节点的中心点坐标。
     * @param octant 指定子节点在八叉体中的位置索引，范围0到7。
     * @param child_size 子节点的立方体空间的大小。
     * @return 子节点的中心点坐标。
     */
    Point3D calculateChildCenter(const Point3D &parent_center, int octant, double child_size)
    {
        double offset = child_size * 0.5;
        return Point3D(
            parent_center.x + ((octant & 4) ? offset : -offset),
            parent_center.y + ((octant & 2) ? offset : -offset),
            parent_center.z + ((octant & 1) ? offset : -offset));
    }

public:
    /**
     * @brief 构建八叉树的对象，指定最大深度和最小点数阈值。
     *
     * 该构造函数提供了八叉树的基本参数，包括最大深度和最小点
     * 数阈值。八叉树的根节点（root）在构造时被初始化为空指针。
     *
     * @param max_depth 八叉树的最大深度（缺省值为6）
     * @param min_points 一个节点中的最小点数阈值（缺省值为5）
     */
    Octree(int max_depth = 6, int min_points = 5)
        : MAX_DEPTH(max_depth), MIN_POINTS(min_points), root(nullptr) {}

    /**
     * @brief 从一组3D点构建八叉树，指定中心点和大小。
     *
     * 该函数递归地构建八叉树，通过将给定的空间分割成八个八面体
     * 直到达到最大深度或某个节点中的点数小于阈值为止。
     *
     * @param points 需要组织到八叉树中的3D点的向量。
     * @param center 当前八叉树节点的中心点。
     * @param size 当前八叉树节点的立方体空间的大小。
     * @param depth 当前八叉树节点的深度级别（缺省值为0）。
     * @return 构建的八叉树的根节点指针。
     */
    OctreeNode *buildOctree(const std::vector<Point3D> &points,
                            const Point3D &center,
                            double size,
                            int depth = 0)
    {
        OctreeNode *node = new OctreeNode(center, size, depth);
        node->points = points;

        if (depth >= MAX_DEPTH || points.size() <= MIN_POINTS)
        {
            return node;
        }

        std::vector<std::vector<Point3D>> child_points(8);
        double child_size = size * 0.5;

        for (const auto &point : points)
        {
            int octant = getOctant(point, node->center);
            child_points[octant].push_back(point);
        }

        bool should_subdivide = false;
        for (const auto &child_point_set : child_points)
        {
            if (!child_point_set.empty() && child_point_set.size() < points.size())
            {
                should_subdivide = true;
                break;
            }
        }

        if (should_subdivide)
        {
            node->children.resize(8);
            for (int i = 0; i < 8; ++i)
            {
                if (!child_points[i].empty())
                {
                    Point3D child_center = calculateChildCenter(center, i, child_size);
                    node->children[i] = buildOctree(child_points[i],
                                                    child_center,
                                                    child_size,
                                                    depth + 1);
                }
            }
        }

        return node;
    }

    /**
     * 递归将八叉树中的每个节点可视化为一个立方体
     * @param node 要可视化的八叉树节点
     * @param renderer  VTK 渲染器
     */
    void addVisualizationCubes(OctreeNode *node, vtkRenderer *renderer)
    {
        if (!node)
            return;

        // 创建立方体
        auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
        cubeSource->SetCenter(node->center.x, node->center.y, node->center.z);
        cubeSource->SetXLength(node->size);
        cubeSource->SetYLength(node->size);
        cubeSource->SetZLength(node->size);

        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(cubeSource->GetOutputPort());

        auto actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetRepresentationToWireframe();

        // 根据深度设置不同的颜色和线条宽度
        double depth_ratio = static_cast<double>(node->depth) / MAX_DEPTH;
        actor->GetProperty()->SetColor(0.0, 1.0 - depth_ratio, depth_ratio); // 颜色从绿到蓝渐变
        actor->GetProperty()->SetLineWidth(3.0 - 2.0 * depth_ratio);         // 线条宽度随深度减小
        actor->GetProperty()->SetOpacity(0.8 - 0.5 * depth_ratio);           // 透明度随深度增加

        renderer->AddActor(actor);

        // 递归处理子节点
        for (auto child : node->children)
        {
            if (child)
            {
                addVisualizationCubes(child, renderer);
            }
        }
    }

    /**
     * @brief 可视化点云数据
     *
     * 该函数将点云数据可视化在给定的渲染器中。它将点云数据
     * 转换为 VTK 的 PolyData 结构，然后使用 VTK 的 VertexGlyphFilter
     * 将点云渲染为球体，并将其添加到渲染器中。
     *
     * @param points 点云数据
     * @param renderer 渲染器
     */
    void visualizePoints(const std::vector<Point3D> &points, vtkRenderer *renderer)
    {
        auto pointSet = vtkSmartPointer<vtkPoints>::New();

        for (const auto &point : points)
        {
            pointSet->InsertNextPoint(point.x, point.y, point.z);
        }

        auto polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(pointSet);

        auto vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        vertexFilter->SetInputData(polyData);
        vertexFilter->Update();

        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(vertexFilter->GetOutputPort());

        auto actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1.0, 0.0, 0.0);        // 点云颜色设为红色
        actor->GetProperty()->SetPointSize(5);                // 增大点的大小
        actor->GetProperty()->SetRenderPointsAsSpheres(true); // 将点渲染为球体

        renderer->AddActor(actor);
    }

    void setRoot(OctreeNode *node) { root = node; }
    OctreeNode *getRoot() { return root; }
};

/**
 * @brief 主函数，生成和可视化点云及其八叉树结构。
 *
 * 该函数通过生成分布在不同聚类中的点云数据，构建对应的八叉树结构，并在
 * 可视化窗口中显示。它设置渲染器和窗口属性，调整相机视角，并启用抗锯齿
 * 以提高显示效果。
 *
 * - 生成具有结构的点云，包括多个聚类。
 * - 构建八叉树以组织和管理点云数据。
 * - 使用VTK库创建渲染器和窗口以进行可视化。
 * - 调整窗口大小、背景颜色、相机位置和视角。
 * - 启用抗锯齿以提高渲染质量。
 * - 启动渲染窗口交互。
 *
 * @return 0 表示成功完成。
 */
int main()
{
    // 生成更有结构的点云
    std::random_device rd;
    std::mt19937 gen(rd());

    // 创建clustered分布的点云
    std::vector<Point3D> points;

    // 生成几个聚类
    std::vector<Point3D> centers = {
        Point3D(-3, -3, -3),
        Point3D(3, 3, 3),
        Point3D(-3, 3, -3),
        Point3D(3, -3, 3)};

    std::normal_distribution<> cluster_dis(0.0, 1.0);

    // 为每个聚类生成点
    for (const auto &center : centers)
    {
        for (int i = 0; i < 250; ++i)
        { // 每个聚类250个点
            Point3D point(
                center.x + cluster_dis(gen),
                center.y + cluster_dis(gen),
                center.z + cluster_dis(gen));
            points.push_back(point);
        }
    }

    // 创建八叉树
    Octree octree(6, 10); // 调整最大深度和最小点数
    Point3D center(0, 0, 0);
    double size = 12.0;
    OctreeNode *root = octree.buildOctree(points, center, size);
    octree.setRoot(root);

    // 创建渲染器和窗口
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    auto renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // 设置渲染器的背景
    renderer->SetBackground(0.2, 0.2, 0.2); // 深灰色背景

    // 添加可视化元素
    octree.addVisualizationCubes(octree.getRoot(), renderer);
    octree.visualizePoints(points, renderer);

    // 设置窗口属性
    renderWindow->SetSize(1200, 900); // 增大窗口尺寸
    renderWindow->SetWindowName("Octree Visualization");

    // 设置相机位置
    renderer->GetActiveCamera()->SetPosition(20, 20, 20);
    renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    renderer->GetActiveCamera()->SetViewUp(0, 0, 1);
    renderer->ResetCamera();

    // 启用抗锯齿
    renderWindow->SetMultiSamples(8);

    // 开始交互
    renderWindowInteractor->Initialize();
    renderWindow->Render();
    renderWindowInteractor->Start();

    return 0;
}