#include "Common.hpp"
#include <core/kpi/PropertyView.hpp>
#include <core/util/gui.hpp>
#include <plugins/gc/Export/IndexedPolygon.hpp>

namespace libcube::UI {

auto PolyDataSurface =
    kpi::StatelessPropertyView<libcube::IndexedPolygon>()
        .setTitle("Index Data")
        .setIcon((const char*)ICON_FA_IMAGE)
        .onDraw([](kpi::PropertyDelegate<IndexedPolygon>& dl) {
          auto& poly = dl.getActive();
          auto& desc = poly.getVcd();
          auto& mesh_data = poly.getMeshData();

          auto draw_p = [&](int i, int j) {
            auto prim = poly.getMatrixPrimitiveIndexedPrimitive(i, j);
            u32 k = 0;
            for (auto& v : prim.mVertices) {
              ImGui::TableNextRow();

              riistudio::util::IDScope v_s(k);

              ImGui::TableSetColumnIndex(1);
              ImGui::Text("%u", k);

              u32 q = 0;
              for (auto& e : poly.getVcd().mAttributes) {
                if (e.second == gx::VertexAttributeType::None)
                  continue;
                ImGui::TableSetColumnIndex(2 + q);
                int data = v.operator[](e.first);
                ImGui::Text("%i", data);
                ++q;
              }
              ++k;
            }
          };

          auto draw_mp = [&](int i) {
            auto& mprim = mesh_data.mMatrixPrimitives[i];
            ImGui::Text("Default Matrix: %i", (int)mprim.mCurrentMatrix);

            const int attrib_cnt = std::count_if(
                desc.mAttributes.begin(), desc.mAttributes.end(),
                [](const auto& e) {
                  return e.second != gx::VertexAttributeType::None;
                });

            const auto table_flags =
                ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable;
            if (ImGui::BeginTable("Vertex data", 2 + attrib_cnt, table_flags)) {
              u32 q = 0;
              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

              ImGui::TableSetColumnIndex(0);
              ImGui::Text("Primitive Index");
              ImGui::TableSetColumnIndex(1);
              ImGui::Text("Vertex Index");
              for (auto& e : poly.getVcd().mAttributes) {
                if (e.second == gx::VertexAttributeType::None)
                  continue;

                ImGui::TableSetColumnIndex(2 + q);

                int type = static_cast<int>(e.first);
                ImGui::TextUnformatted(vertexAttribNamesArray[type]);
                ++q;
              }
              static const std::array<std::string, 8> prim_types{
                  "Quads",        "QuadStrips", "Triangles",  "TriangleStrips",
                  "TriangleFans", "Lines",      "LineStrips", "Points"};

              for (int j = 0; j < poly.getMatrixPrimitiveNumIndexedPrimitive(i);
                   ++j) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                bool open = ImGui::TreeNodeEx(
                    (std::string("#") + std::to_string(j) + " (" +
                     prim_types[static_cast<int>(
                         poly.getMatrixPrimitiveIndexedPrimitive(i, j).mType)] +
                     ")")
                        .c_str(),
                    ImGuiTreeNodeFlags_SpanFullWidth |
                        ImGuiTreeNodeFlags_DefaultOpen);
                if (open) {
                  draw_p(i, j);
                  ImGui::TreePop();
                }
              }

              ImGui::EndTable();
            }
          };

          if (ImGui::BeginTabBar("Matrix Primitives")) {
            for (int i = 0; i < poly.getMeshData().mMatrixPrimitives.size();
                 ++i) {
              if (ImGui::BeginTabItem(
                      (std::string("Matrix Prim: ") + std::to_string(i))
                          .c_str())) {
                draw_mp(i);
                ImGui::EndTabItem();
              }
            }
            ImGui::EndTabBar();
          }
        });

} // namespace libcube::UI
