#pragma once

#include <Renderer/RenderItem.hpp>
#include <Renderer/Sampler.hpp>
#include <Renderer/Shader.hpp>

#include <cstdint>
#include <vector>

class PresetState;
class PerFrameContext;
class PerPixelContext;
class MilkdropShader;

/**
 * @brief The "per-pixel" transformation mesh.
 *
 * This mesh is responsible for most of the motion types in presets. Each mesh vertex
 * is transposed (also scaled, from the center) or rotated to create a frame-by-frame motion.
 * Fragment shader interpolation is then used to create smooth transitions in the space
 * between the grid points.
 *
 * A higher resolution grid means better quality, especially for rotations, but also quickly
 * increases the CPU usage as the per-pixel expression needs to be run for every grid point.
 *
 * The mesh size can be changed between frames, the class will reallocate the buffers if needed.
 */
class PerPixelMesh : public RenderItem
{
public:
    PerPixelMesh();

    void InitVertexAttrib() override;

    void CompileWarpShader(const PresetState& presetState);

    /**
     * @brief Renders the transformation mesh.
     * @param presetState The preset state to retrieve the configuration values from.
     * @param presetPerFrameContext The per-frame context to retrieve the initial vars from.
     * @param perPixelContext The per-pixel code context to use.
     */
    void Draw(const PresetState& presetState,
              const PerFrameContext& perFrameContext,
              PerPixelContext& perPixelContext);

private:
    /**
     * @brief Initializes the vertex array and fills in static data if needed.
     *
     * The vertices will be reallocated if the grid size has changed. If either this happened,
     * or the viewport size changed, the static values will be recalculated.
     *
     * @param presetState The preset state to retrieve the configuration values from.
     */
    void InitializeMesh(const PresetState& presetState);

    /**
     * @brief Executes the per-pixel code and calculates the u/v coordinates.
     * The x/y coordinates are either a static grid or computed by the per-vertex expression.
     * @param presetState The preset state to retrieve the configuration values from.
     * @param presetPerFrameContext The per-frame context to retrieve the initial vars from.
     * @param perPixelContext The per-pixel code context to use.
     */
    void CalculateMesh(const PresetState& presetState,
                       const PerFrameContext& perFrameContext,
                       PerPixelContext& perPixelContext);

    /**
     * @brief Draws the mesh without a warp shader.
     * Since OpenGL doesn't support drawing without shaders, this function uses the standard
     * textured shader also used for textured shapes.
     */
    void WarpedBlit(const PresetState& presetState, const PerFrameContext& perFrameContext);

    /**
     * @brief Draws the mesh using a warp shader.
     * In Milkdrop 2 presets, this is either a preset-defined shader or a default warp
     * shader.
     * @param warpShader
     */
    void BlitShader(MilkdropShader* warpShader);

    struct MeshVertex {
        float x{};
        float y{};
        float radius{};
        float angle{};

        float zoom{};
        float zoomExp{};
        float rot{};
        float warp{};

        float centerX{};
        float centerY{};

        float distanceX{};
        float distanceY{};

        float stretchX{};
        float stretchY{};
    };

    int m_gridSizeX{};
    int m_gridSizeY{};

    int m_viewportWidth{};
    int m_viewportHeight{};

    GLuint m_vebID{0}; //!< This RenderItem's vertex element buffer ID

    std::vector<MeshVertex> m_vertices; //!< The calculated mesh vertices.

    std::vector<int> m_listIndices;         //!< List of vertex indices to render.
    std::vector<MeshVertex> m_drawVertices; //!< Temp data buffer for the vertices to be drawn.

    Shader m_perPixelMeshShader; //!< Special shader which calculates the per-pixel UV coordinates.
    std::unique_ptr<MilkdropShader> m_warpShader;           //!< The warp shader. Either preset-defined or a default shader.
    Sampler m_perPixelSampler{GL_CLAMP_TO_EDGE, GL_LINEAR}; //!< The main texture sampler.
};