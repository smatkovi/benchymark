/*
 * SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "VulkanBench.h"

#include <QElapsedTimer>
#include <QFile>
#include <QMetaObject>
#include <QPainter>
#include <algorithm>
#include <dlfcn.h>
#include <vulkan/vulkan.h>

#include "vulkan-spv.h"

// ---- die Einsprungstellen, zur Laufzeit geholt --------------------------
// Gebunden wird nichts: auf einem Geraet ohne libhybris-libvulkan soll die
// App starten und die Stufe nur sagen, dass es hier kein Vulkan gibt.
#define VK_LISTE(X)                                                            \
    X(vkCreateInstance) X(vkDestroyInstance) X(vkEnumeratePhysicalDevices)     \
    X(vkGetPhysicalDeviceProperties) X(vkGetPhysicalDeviceQueueFamilyProperties)\
    X(vkGetPhysicalDeviceMemoryProperties) X(vkCreateDevice) X(vkDestroyDevice)\
    X(vkGetDeviceQueue) X(vkDeviceWaitIdle) X(vkCreateCommandPool)             \
    X(vkDestroyCommandPool) X(vkAllocateCommandBuffers) X(vkBeginCommandBuffer)\
    X(vkEndCommandBuffer) X(vkResetCommandBuffer) X(vkCreateFence)             \
    X(vkDestroyFence) X(vkWaitForFences) X(vkResetFences) X(vkQueueSubmit)     \
    X(vkCreateQueryPool) X(vkDestroyQueryPool) X(vkCmdResetQueryPool)          \
    X(vkCmdWriteTimestamp) X(vkGetQueryPoolResults) X(vkCreateImage)           \
    X(vkDestroyImage) X(vkGetImageMemoryRequirements) X(vkAllocateMemory)      \
    X(vkFreeMemory) X(vkBindImageMemory) X(vkCreateImageView)                  \
    X(vkDestroyImageView) X(vkCreateRenderPass) X(vkDestroyRenderPass)         \
    X(vkCreateFramebuffer) X(vkDestroyFramebuffer) X(vkCreateBuffer)           \
    X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) X(vkBindBufferMemory)  \
    X(vkMapMemory) X(vkUnmapMemory) X(vkCreateShaderModule)                    \
    X(vkDestroyShaderModule) X(vkCreatePipelineLayout)                         \
    X(vkDestroyPipelineLayout) X(vkCreateGraphicsPipelines) X(vkDestroyPipeline)\
    X(vkCmdBeginRenderPass) X(vkCmdEndRenderPass) X(vkCmdBindPipeline)         \
    X(vkCmdPushConstants) X(vkCmdDraw) X(vkCmdCopyImageToBuffer)

namespace {

struct Api {
#define EINTRAG(n) PFN_##n n = nullptr;
    VK_LISTE(EINTRAG)
#undef EINTRAG
    void *haus = nullptr;

    bool laden(QString *grund)
    {
        haus = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!haus)
            haus = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
        if (!haus) {
            *grund = QStringLiteral("libvulkan.so.1 fehlt");
            return false;
        }
#define HOLEN(n)                                                               \
    n = reinterpret_cast<PFN_##n>(dlsym(haus, #n));                            \
    if (!n) { *grund = QStringLiteral(#n " fehlt"); return false; }
        VK_LISTE(HOLEN)
#undef HOLEN
        return true;
    }

    void zu()
    {
        if (haus) {
            dlclose(haus);
            haus = nullptr;
        }
    }
};

// Alles, was ein Lauf an Vulkan-Gegenstaenden haelt, damit das Aufraeumen
// an einer Stelle steht.
struct Lauf {
    Api v;
    VkInstance inst = VK_NULL_HANDLE;
    VkPhysicalDevice pgeraet = VK_NULL_HANDLE;
    VkDevice dev = VK_NULL_HANDLE;
    VkQueue schlange = VK_NULL_HANDLE;
    VkCommandPool lager = VK_NULL_HANDLE;
    VkCommandBuffer befehle = VK_NULL_HANDLE;
    VkCommandBuffer holer = VK_NULL_HANDLE;
    VkFence zaun = VK_NULL_HANDLE;
    VkQueryPool fragen = VK_NULL_HANDLE;
    VkImage ziel = VK_NULL_HANDLE;
    VkDeviceMemory zielspeicher = VK_NULL_HANDLE;
    VkImageView zielsicht = VK_NULL_HANDLE;
    VkRenderPass gang = VK_NULL_HANDLE;
    VkFramebuffer rahmen = VK_NULL_HANDLE;
    VkBuffer lese = VK_NULL_HANDLE;
    VkDeviceMemory lesespeicher = VK_NULL_HANDLE;
    void *lesekarte = nullptr;
    VkPipelineLayout belegung = VK_NULL_HANDLE;
    VkPipeline strecke = VK_NULL_HANDLE;
    float zeitstempel_ns = 1.0f;
    bool hatZeitstempel = false;

    uint32_t speichertyp(uint32_t maske, VkMemoryPropertyFlags will) const
    {
        VkPhysicalDeviceMemoryProperties mp;
        v.vkGetPhysicalDeviceMemoryProperties(pgeraet, &mp);
        for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
            if ((maske & (1u << i)) && (mp.memoryTypes[i].propertyFlags & will) == will)
                return i;
        return UINT32_MAX;
    }

    void abbauen()
    {
        if (dev) {
            v.vkDeviceWaitIdle(dev);
            if (strecke) v.vkDestroyPipeline(dev, strecke, nullptr);
            if (belegung) v.vkDestroyPipelineLayout(dev, belegung, nullptr);
            if (lesekarte) v.vkUnmapMemory(dev, lesespeicher);
            if (lese) v.vkDestroyBuffer(dev, lese, nullptr);
            if (lesespeicher) v.vkFreeMemory(dev, lesespeicher, nullptr);
            if (rahmen) v.vkDestroyFramebuffer(dev, rahmen, nullptr);
            if (gang) v.vkDestroyRenderPass(dev, gang, nullptr);
            if (zielsicht) v.vkDestroyImageView(dev, zielsicht, nullptr);
            if (ziel) v.vkDestroyImage(dev, ziel, nullptr);
            if (zielspeicher) v.vkFreeMemory(dev, zielspeicher, nullptr);
            if (fragen) v.vkDestroyQueryPool(dev, fragen, nullptr);
            if (zaun) v.vkDestroyFence(dev, zaun, nullptr);
            if (lager) v.vkDestroyCommandPool(dev, lager, nullptr);
            v.vkDestroyDevice(dev, nullptr);
            dev = VK_NULL_HANDLE;
        }
        if (inst)
            v.vkDestroyInstance(inst, nullptr);
        inst = VK_NULL_HANDLE;
        v.zu();
    }
};

VkShaderModule modul(Lauf &l, const uint32_t *worte, size_t bytes)
{
    VkShaderModuleCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = bytes;
    ci.pCode = worte;
    VkShaderModule m = VK_NULL_HANDLE;
    if (l.v.vkCreateShaderModule(l.dev, &ci, nullptr, &m) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    return m;
}

} // namespace

// ---- VulkanView ---------------------------------------------------------
VulkanView::VulkanView(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setOpaquePainting(true);
}

void VulkanView::paint(QPainter *painter)
{
    QImage bild;
    {
        QMutexLocker lock(&m_lock);
        bild = m_bild;
    }
    if (bild.isNull()) {
        painter->fillRect(boundingRect(), QColor(8, 14, 30));
        return;
    }
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(boundingRect(), bild);
}

void VulkanView::setPicture(const QImage &bild)
{
    {
        QMutexLocker lock(&m_lock);
        m_bild = bild;
    }
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

// ---- VulkanEngine -------------------------------------------------------
VulkanEngine::VulkanEngine(QObject *parent) : QObject(parent) {}

VulkanEngine::~VulkanEngine() { stop(); }

void VulkanEngine::melde(int phase, const QString &grund)
{
    {
        QMutexLocker lock(&m_lock);
        m_phase = phase;
        if (!grund.isEmpty())
            m_reason = grund;
    }
    QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
}

void VulkanEngine::start(int w, int h, int preheatMs, int measureMs)
{
    stop();
    if (w < 16 || h < 16) {
        melde(-1, QStringLiteral("Flaeche zu klein"));
        return;
    }
    m_stop.store(0);
    m_thread = new FunctionThread([this, w, h, preheatMs, measureMs] {
        work(w, h, preheatMs, measureMs);
    });
    m_thread->start();
}

void VulkanEngine::stop()
{
    m_stop.store(1);
    if (m_thread) {
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
}

void VulkanEngine::work(int w, int h, int preheatMs, int measureMs)
{
    Lauf l;
    QString grund;
    if (!l.v.laden(&grund)) {
        melde(-1, grund);
        return;
    }

    VkApplicationInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "harbour-benchymark";
    ai.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &ai;
    VkResult r = l.v.vkCreateInstance(&ici, nullptr, &l.inst);
    if (r != VK_SUCCESS) {
        melde(-1, QStringLiteral("vkCreateInstance %1").arg(int(r)));
        l.abbauen();
        return;
    }

    uint32_t anz = 0;
    l.v.vkEnumeratePhysicalDevices(l.inst, &anz, nullptr);
    if (!anz) {
        melde(-1, QStringLiteral("kein Vulkan-Geraet"));
        l.abbauen();
        return;
    }
    QVector<VkPhysicalDevice> geraete;      // nicht (int(anz)) -- das waere
    geraete.resize(int(anz));              // eine Funktionsdeklaration
    l.v.vkEnumeratePhysicalDevices(l.inst, &anz, geraete.data());
    l.pgeraet = geraete[0];

    VkPhysicalDeviceProperties pe;
    l.v.vkGetPhysicalDeviceProperties(l.pgeraet, &pe);
    l.zeitstempel_ns = pe.limits.timestampPeriod;
    {
        QMutexLocker lock(&m_lock);
        m_device = QString::fromLatin1(pe.deviceName);
        m_api = QStringLiteral("%1.%2.%3").arg(VK_VERSION_MAJOR(pe.apiVersion))
                    .arg(VK_VERSION_MINOR(pe.apiVersion)).arg(VK_VERSION_PATCH(pe.apiVersion));
        m_driver = QStringLiteral("%1.%2.%3").arg(VK_VERSION_MAJOR(pe.driverVersion))
                    .arg(VK_VERSION_MINOR(pe.driverVersion)).arg(VK_VERSION_PATCH(pe.driverVersion));
    }

    uint32_t qn = 0;
    l.v.vkGetPhysicalDeviceQueueFamilyProperties(l.pgeraet, &qn, nullptr);
    QVector<VkQueueFamilyProperties> qf;
    qf.resize(int(qn));
    l.v.vkGetPhysicalDeviceQueueFamilyProperties(l.pgeraet, &qn, qf.data());
    uint32_t familie = UINT32_MAX;
    for (uint32_t i = 0; i < qn; i++)
        if (qf[int(i)].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            familie = i;
            l.hatZeitstempel = qf[int(i)].timestampValidBits > 0;
            break;
        }
    if (familie == UINT32_MAX) {
        melde(-1, QStringLiteral("keine Grafikschlange"));
        l.abbauen();
        return;
    }

    const float rang = 1.0f;
    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = familie;
    qci.queueCount = 1;
    qci.pQueuePriorities = &rang;
    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    if (l.v.vkCreateDevice(l.pgeraet, &dci, nullptr, &l.dev) != VK_SUCCESS) {
        melde(-1, QStringLiteral("vkCreateDevice scheitert"));
        l.abbauen();
        return;
    }
    l.v.vkGetDeviceQueue(l.dev, familie, 0, &l.schlange);

    VkCommandPoolCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pci.queueFamilyIndex = familie;
    l.v.vkCreateCommandPool(l.dev, &pci, nullptr, &l.lager);
    VkCommandBufferAllocateInfo bai = {};
    bai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bai.commandPool = l.lager;
    bai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bai.commandBufferCount = 1;
    l.v.vkAllocateCommandBuffers(l.dev, &bai, &l.befehle);
    l.v.vkAllocateCommandBuffers(l.dev, &bai, &l.holer);
    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    l.v.vkCreateFence(l.dev, &fci, nullptr, &l.zaun);
    VkQueryPoolCreateInfo qpi = {};
    qpi.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    qpi.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpi.queryCount = 2;
    if (l.hatZeitstempel)
        l.v.vkCreateQueryPool(l.dev, &qpi, nullptr, &l.fragen);

    // Ziel: ein Bild im Speicher in der Groesse der Stufenflaeche
    VkImageCreateInfo imi = {};
    imi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imi.imageType = VK_IMAGE_TYPE_2D;
    imi.format = VK_FORMAT_R8G8B8A8_UNORM;
    imi.extent.width = uint32_t(w);
    imi.extent.height = uint32_t(h);
    imi.extent.depth = 1;
    imi.mipLevels = 1;
    imi.arrayLayers = 1;
    imi.samples = VK_SAMPLE_COUNT_1_BIT;
    imi.tiling = VK_IMAGE_TILING_OPTIMAL;
    imi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (l.v.vkCreateImage(l.dev, &imi, nullptr, &l.ziel) != VK_SUCCESS) {
        melde(-1, QStringLiteral("vkCreateImage scheitert"));
        l.abbauen();
        return;
    }
    VkMemoryRequirements be;
    l.v.vkGetImageMemoryRequirements(l.dev, l.ziel, &be);
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = be.size;
    mai.memoryTypeIndex = l.speichertyp(be.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (mai.memoryTypeIndex == UINT32_MAX ||
        l.v.vkAllocateMemory(l.dev, &mai, nullptr, &l.zielspeicher) != VK_SUCCESS) {
        melde(-1, QStringLiteral("kein Bildspeicher"));
        l.abbauen();
        return;
    }
    l.v.vkBindImageMemory(l.dev, l.ziel, l.zielspeicher, 0);

    VkImageViewCreateInfo vci = {};
    vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vci.image = l.ziel;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = VK_FORMAT_R8G8B8A8_UNORM;
    vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vci.subresourceRange.levelCount = 1;
    vci.subresourceRange.layerCount = 1;
    l.v.vkCreateImageView(l.dev, &vci, nullptr, &l.zielsicht);

    VkAttachmentDescription ab = {};
    ab.format = VK_FORMAT_R8G8B8A8_UNORM;
    ab.samples = VK_SAMPLE_COUNT_1_BIT;
    ab.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ab.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ab.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ab.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ab.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ab.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    VkAttachmentReference ar = {};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription sp = {};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &ar;
    VkSubpassDependency dep[2] = {};
    dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dep[0].dstSubpass = 0;
    dep[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dep[1].srcSubpass = 0;
    dep[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dep[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    VkRenderPassCreateInfo rci = {};
    rci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rci.attachmentCount = 1;
    rci.pAttachments = &ab;
    rci.subpassCount = 1;
    rci.pSubpasses = &sp;
    rci.dependencyCount = 2;
    rci.pDependencies = dep;
    l.v.vkCreateRenderPass(l.dev, &rci, nullptr, &l.gang);

    VkFramebufferCreateInfo fbi = {};
    fbi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbi.renderPass = l.gang;
    fbi.attachmentCount = 1;
    fbi.pAttachments = &l.zielsicht;
    fbi.width = uint32_t(w);
    fbi.height = uint32_t(h);
    fbi.layers = 1;
    l.v.vkCreateFramebuffer(l.dev, &fbi, nullptr, &l.rahmen);

    // Lesepuffer fuer die Vorschau
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = VkDeviceSize(w) * h * 4;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    l.v.vkCreateBuffer(l.dev, &bci, nullptr, &l.lese);
    l.v.vkGetBufferMemoryRequirements(l.dev, l.lese, &be);
    mai.allocationSize = be.size;
    mai.memoryTypeIndex = l.speichertyp(be.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (mai.memoryTypeIndex != UINT32_MAX &&
        l.v.vkAllocateMemory(l.dev, &mai, nullptr, &l.lesespeicher) == VK_SUCCESS) {
        l.v.vkBindBufferMemory(l.dev, l.lese, l.lesespeicher, 0);
        l.v.vkMapMemory(l.dev, l.lesespeicher, 0, VK_WHOLE_SIZE, 0, &l.lesekarte);
    }

    // Strecke: Vollbild-Dreieck, der Shader von SHADER HEAVY
    VkShaderModule mv = modul(l, kSpvVoll, sizeof kSpvVoll);
    VkShaderModule mf = modul(l, kSpvWolke, sizeof kSpvWolke);
    if (!mv || !mf) {
        melde(-1, QStringLiteral("Shader nicht angenommen"));
        l.abbauen();
        return;
    }
    VkPushConstantRange schub = {};
    schub.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    schub.size = sizeof(float);
    VkPipelineLayoutCreateInfo pli = {};
    pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.pushConstantRangeCount = 1;
    pli.pPushConstantRanges = &schub;
    l.v.vkCreatePipelineLayout(l.dev, &pli, nullptr, &l.belegung);

    VkPipelineShaderStageCreateInfo stufen[2] = {};
    stufen[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stufen[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stufen[0].module = mv;
    stufen[0].pName = "main";
    stufen[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stufen[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stufen[1].module = mf;
    stufen[1].pName = "main";
    VkPipelineVertexInputStateCreateInfo ein = {};
    ein.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkPipelineInputAssemblyStateCreateInfo zus = {};
    zus.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    zus.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkViewport sicht = { 0, 0, float(w), float(h), 0.0f, 1.0f };
    VkRect2D schnitt = { { 0, 0 }, { uint32_t(w), uint32_t(h) } };
    VkPipelineViewportStateCreateInfo vs = {};
    vs.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vs.viewportCount = 1;
    vs.pViewports = &sicht;
    vs.scissorCount = 1;
    vs.pScissors = &schnitt;
    VkPipelineRasterizationStateCreateInfo ra = {};
    ra.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ra.polygonMode = VK_POLYGON_MODE_FILL;
    ra.cullMode = VK_CULL_MODE_NONE;
    ra.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    ra.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineColorBlendAttachmentState mz = {};
    mz.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo mb = {};
    mb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    mb.attachmentCount = 1;
    mb.pAttachments = &mz;
    VkGraphicsPipelineCreateInfo gpi = {};
    gpi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpi.stageCount = 2;
    gpi.pStages = stufen;
    gpi.pVertexInputState = &ein;
    gpi.pInputAssemblyState = &zus;
    gpi.pViewportState = &vs;
    gpi.pRasterizationState = &ra;
    gpi.pMultisampleState = &ms;
    gpi.pColorBlendState = &mb;
    gpi.layout = l.belegung;
    gpi.renderPass = l.gang;
    VkResult pr = l.v.vkCreateGraphicsPipelines(l.dev, VK_NULL_HANDLE, 1, &gpi, nullptr, &l.strecke);
    l.v.vkDestroyShaderModule(l.dev, mv, nullptr);
    l.v.vkDestroyShaderModule(l.dev, mf, nullptr);
    if (pr != VK_SUCCESS) {
        melde(-1, QStringLiteral("Strecke %1").arg(int(pr)));
        l.abbauen();
        return;
    }

    // ---- messen ---------------------------------------------------------
    QVector<double> zeiten;
    QElapsedTimer uhr;
    uhr.start();
    QElapsedTimer vorschau;
    vorschau.start();
    const qint64 endeVorheizen = preheatMs;
    const qint64 ende = preheatMs + measureMs;
    melde(preheatMs > 0 ? 1 : 2);

    while (!m_stop.load() && uhr.elapsed() < ende) {
        const bool misst = uhr.elapsed() >= endeVorheizen;
        // t wie in der Stufe SHADER HEAVY: die QML-Animation laeuft von 0 bis
        // 10000 in 10000 Sekunden, also eine Einheit je Sekunde.
        const float t = float(uhr.elapsed()) / 1000.0f;

        l.v.vkResetCommandBuffer(l.befehle, 0);
        VkCommandBufferBeginInfo cbi = {};
        cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        l.v.vkBeginCommandBuffer(l.befehle, &cbi);
        if (l.fragen) {
            l.v.vkCmdResetQueryPool(l.befehle, l.fragen, 0, 2);
            l.v.vkCmdWriteTimestamp(l.befehle, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, l.fragen, 0);
        }
        VkClearValue klar = {};
        klar.color.float32[0] = 0.02f;
        klar.color.float32[1] = 0.04f;
        klar.color.float32[2] = 0.10f;
        klar.color.float32[3] = 1.0f;
        VkRenderPassBeginInfo rbi = {};
        rbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rbi.renderPass = l.gang;
        rbi.framebuffer = l.rahmen;
        rbi.renderArea = schnitt;
        rbi.clearValueCount = 1;
        rbi.pClearValues = &klar;
        l.v.vkCmdBeginRenderPass(l.befehle, &rbi, VK_SUBPASS_CONTENTS_INLINE);
        l.v.vkCmdBindPipeline(l.befehle, VK_PIPELINE_BIND_POINT_GRAPHICS, l.strecke);
        l.v.vkCmdPushConstants(l.befehle, l.belegung, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(float), &t);
        l.v.vkCmdDraw(l.befehle, 3, 1, 0, 0);
        l.v.vkCmdEndRenderPass(l.befehle);
        if (l.fragen)
            l.v.vkCmdWriteTimestamp(l.befehle, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, l.fragen, 1);
        l.v.vkEndCommandBuffer(l.befehle);

        QElapsedTimer wand;
        wand.start();
        VkSubmitInfo si = {};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &l.befehle;
        if (l.v.vkQueueSubmit(l.schlange, 1, &si, l.zaun) != VK_SUCCESS) {
            melde(-1, QStringLiteral("vkQueueSubmit scheitert"));
            l.abbauen();
            return;
        }
        l.v.vkWaitForFences(l.dev, 1, &l.zaun, VK_TRUE, 5000000000ull);
        l.v.vkResetFences(l.dev, 1, &l.zaun);
        double ms = double(wand.nsecsElapsed()) / 1.0e6;
        if (l.fragen) {
            uint64_t werte[2] = { 0, 0 };
            if (l.v.vkGetQueryPoolResults(l.dev, l.fragen, 0, 2, sizeof werte, werte,
                                          sizeof(uint64_t), VK_QUERY_RESULT_64_BIT) == VK_SUCCESS &&
                werte[1] > werte[0])
                ms = double(werte[1] - werte[0]) * double(l.zeitstempel_ns) / 1.0e6;
        }

        if (misst) {
            zeiten.append(ms);
            QVector<double> sortiert = zeiten;
            std::sort(sortiert.begin(), sortiert.end());
            {
                QMutexLocker lock(&m_lock);
                m_frames = sortiert.size();
                m_gpuMs = sortiert[sortiert.size() / 2];
                m_bestMs = sortiert.first();
                m_phase = 2;
            }
            QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
        }

        // einmal in der Sekunde ein Bild fuer die Vorschau holen
        if (l.lesekarte && m_view && vorschau.elapsed() >= 1000) {
            vorschau.restart();
            VkCommandBufferBeginInfo hbi = {};
            hbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            hbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            l.v.vkResetCommandBuffer(l.holer, 0);
            l.v.vkBeginCommandBuffer(l.holer, &hbi);
            VkBufferImageCopy kopie = {};
            kopie.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            kopie.imageSubresource.layerCount = 1;
            kopie.imageExtent.width = uint32_t(w);
            kopie.imageExtent.height = uint32_t(h);
            kopie.imageExtent.depth = 1;
            l.v.vkCmdCopyImageToBuffer(l.holer, l.ziel, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       l.lese, 1, &kopie);
            l.v.vkEndCommandBuffer(l.holer);
            VkSubmitInfo hs = {};
            hs.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            hs.commandBufferCount = 1;
            hs.pCommandBuffers = &l.holer;
            l.v.vkQueueSubmit(l.schlange, 1, &hs, l.zaun);
            l.v.vkWaitForFences(l.dev, 1, &l.zaun, VK_TRUE, 5000000000ull);
            l.v.vkResetFences(l.dev, 1, &l.zaun);
            QImage bild(reinterpret_cast<const uchar *>(l.lesekarte), w, h,
                        QImage::Format_RGBA8888);
            // halbe Kantenlaenge reicht fuer die Vorschau und kostet wenig
            m_view->setPicture(bild.scaled(w / 2, h / 2, Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation));
        }
    }

    {
        QMutexLocker lock(&m_lock);
        m_phase = 3;
    }
    QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
    l.abbauen();
}
