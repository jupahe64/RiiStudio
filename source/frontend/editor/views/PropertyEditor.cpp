#include "PropertyEditor.hpp"
#include <core/3d/Material.hpp>           // lib3d::Material
#include <imgui/imgui.h>                  // ImGui::Text
#include <vendor/fa5/IconsFontAwesome5.h> // (const char*)ICON_FA_EXCLAMATION_TRIANGLE

namespace riistudio::frontend {

class PropertyEditor : public StudioWindow {
public:
  PropertyEditor(kpi::History& host, kpi::INode& root, kpi::IObject*& active,
                 EditorWindow& ed);
  ~PropertyEditor();

private:
  void draw_() override;

  enum class Mode {
    Tabs,     //!< Standard horizontal tabs
    VertTabs, //!< Tree of tabs on the left
    Headers   //!< Use collapsing headers
  };
  Mode mMode = Mode::Tabs;
  int mActiveTab = 0;
  std::vector<bool> tab_filter;

  EditorWindow& ed;
  kpi::History& mHost;
  kpi::INode& mRoot;
  kpi::IObject*& mActive;

  kpi::PropertyViewStateHolder state_holder;
};

template <typename T>
static void gatherSelected(std::vector<kpi::IObject*>& tmp,
                           kpi::ICollection& folder, T pred) {
  for (int i = 0; i < folder.size(); ++i) {
    auto* obj = folder.atObject(i);
    if (folder.isSelected(i) && pred(obj)) {
      tmp.push_back(obj);
    }

    auto* col = dynamic_cast<kpi::INode*>(obj);
    if (col != nullptr) {
      for (int j = 0; j < col->numFolders(); ++j) {
        gatherSelected(tmp, *col->folderAt(i), pred);
      }
    }
  }
}

PropertyEditor::PropertyEditor(kpi::History& host, kpi::INode& root,
                               kpi::IObject*& active, EditorWindow& ed)
    : StudioWindow("Property Editor"), mHost(host), mRoot(root),
      mActive(active), ed(ed) {
  setWindowFlag(ImGuiWindowFlags_MenuBar);
}

PropertyEditor::~PropertyEditor() { state_holder.garbageCollect(); }

void PropertyEditor::draw_() {
  auto& manager = kpi::PropertyViewManager::getInstance();

  if (mActive == nullptr) {
    ImGui::Text("Nothing is selected.");
    return;
  }

  if (lib3d::Material* mat = dynamic_cast<lib3d::Material*>(mActive);
      mat != nullptr && mat->isShaderError) {
    ImGui::SetWindowFontScale(2.0f);
    ImVec4 warnColor{1.0f, 0.0f, 0.0f, 1.0f};
    ImGui::TextColored(warnColor, "[WARNING] Invalid shader!");
    ImGui::TextColored(warnColor, mat->shaderError.c_str());
    ImGui::SetWindowFontScale(1.0f);
  }

  const auto draw_tab_widget = [&](bool compact = false) {
    int mode = static_cast<int>(mMode);
    if (compact)
      ImGui::PushItemWidth(150);
    ImGui::Combo(compact ? "##Property Mode" : "Property Mode", &mode,
                 "Tabs\0Vertical Tabs\0Headers\0");
    if (compact)
      ImGui::PopItemWidth();
    mMode = static_cast<Mode>(mode);
  };

  if (ImGui::BeginPopupContextWindow()) {
    draw_tab_widget();
    ImGui::EndPopup();
  }
  if (ImGui::BeginMenuBar()) {
    draw_tab_widget(true);
    ImGui::EndMenuBar();
  }

  std::vector<kpi::IObject*> selected;
  for (int i = 0; i < mRoot.numFolders();  ++i) {
    gatherSelected(selected, *mRoot.folderAt(i), [&](kpi::IObject* node) {
      return typeid(*node).name() == typeid(mActive).name(); // return node->mType == mActive->mType;
    });
  }

  kpi::IPropertyView* activeTab = nullptr;

  if (selected.empty()) {
    ImGui::Text((const char*)ICON_FA_EXCLAMATION_TRIANGLE
                " Active selection and multiselection desynced."
                " This shouldn't happen.");
    selected.push_back(mActive);
  }
  ImGui::Text("%s %s (%u)", mActive->getName().c_str(),
              selected.size() > 1 ? "..." : "",
              static_cast<u32>(selected.size()));

  if (mMode == Mode::Tabs) {
    if (ImGui::BeginTabBar("Pane")) {
      int i = 0;
      std::string title;
      manager.forEachView(
          [&](kpi::IPropertyView& view) {
            // const bool sel = mActiveTab == i;

            title.clear();
            title += view.getIcon();
            title += " ";
            title += view.getName();

            if (ImGui::BeginTabItem(title.c_str())) {
              mActiveTab = i;
              activeTab = &view;
              ImGui::EndTabItem();
            }

            ++i;
          },
          *mActive);
      ImGui::EndTabBar();
    }

    if (activeTab == nullptr) {
      mActiveTab = 0;

      ImGui::Text("Invalid Pane");
      return;
    }

    activeTab->draw(*mActive, selected, mHost, mRoot, state_holder, &ed);
  } else if (mMode == Mode::VertTabs) {
    ImGui::BeginChild("Left", ImVec2(120, 0), true);
    int i = 0;
    std::string title;
    manager.forEachView(
        [&](kpi::IPropertyView& view) {
          const bool sel = mActiveTab == i;

          title.clear();
          title += view.getIcon();
          title += " ";
          title += view.getName();

          if (ImGui::Selectable(title.c_str(), sel) || sel) {
            mActiveTab = i;
            activeTab = &view;
          }

          ++i;
        },
        *mActive);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Right", ImGui::GetContentRegionAvail(), true);
    {
      if (activeTab == nullptr) {
        mActiveTab = 0;

        ImGui::Text("Invalid Pane");
      } else {
        activeTab->draw(*mActive, selected, mHost, mRoot, state_holder, &ed);
      }
    }
    ImGui::EndChild();

  } else if (mMode == Mode::Headers) {
    std::string title;
    int i = 0;
    manager.forEachView([&](kpi::IPropertyView& view) { ++i; }, *mActive);
    const int num_headers = i;

    if (tab_filter.size() != num_headers) {
      tab_filter.resize(num_headers);
      std::fill(tab_filter.begin(), tab_filter.end(), true);
    }

    i = 0;
    manager.forEachView(
        [&](kpi::IPropertyView& view) {
          title.clear();
          title += view.getIcon();
          title += " ";
          title += view.getName();
          // TODO: >
          bool tmp = tab_filter[i];
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
          ImGui::Checkbox(title.c_str(), &tmp);
          ImGui::PopStyleVar();
          tab_filter[i] = tmp;

          ++i;
        },
        *mActive);

    i = 0;
    manager.forEachView(
        [&](kpi::IPropertyView& view) {
          if (!tab_filter[i++])
            return;

          title.clear();
          title += view.getIcon();
          title += " ";
          title += view.getName();
          if (ImGui::CollapsingHeader(title.c_str(),
                                      ImGuiTreeNodeFlags_DefaultOpen)) {
            view.draw(*mActive, selected, mHost, mRoot, state_holder, &ed);
          }
        },
        *mActive);
  } else {
    ImGui::Text("Unknown mode");
  }

  state_holder.garbageCollect();
}

std::unique_ptr<StudioWindow> MakePropertyEditor(kpi::History& host,
                                                 kpi::INode& root,
                                                 kpi::IObject*& active,
                                                 EditorWindow& ed) {
  return std::make_unique<PropertyEditor>(host, root, active, ed);
}

} // namespace riistudio::frontend
