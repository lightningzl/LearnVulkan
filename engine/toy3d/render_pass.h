#pragma once
#include "render_pass_base.h"
#include "vulkan/vulkan.hpp"

namespace toy3d
{
	enum
	{
		_main_camera_pass_gbuffer_a = 0,
		_main_camera_pass_gbuffer_b = 1,
		_main_camera_pass_gbuffer_c = 2,
		_main_camera_pass_backup_buffer_odd = 3,
		_main_camera_pass_backup_buffer_even = 4,
		_main_camera_pass_post_process_buffer_odd = 5,
		_main_camera_pass_post_process_buffer_even = 6,
		_main_camera_pass_depth = 7,
		_main_camera_pass_swap_chain_image = 8,
		_main_camera_pass_custom_attachment_count = 5,
		_main_camera_pass_post_process_attachment_count = 2,
		_main_camera_pass_attachment_count = 9,
	};

	enum
	{
		_main_camera_subpass_basepass = 0,
		_main_camera_subpass_deferred_lighting,
		_main_camera_subpass_forward_lighting,
		_main_camera_subpass_tone_mapping,
		_main_camera_subpass_color_grading,
		_main_camera_subpass_fxaa,
		_main_camera_subpass_ui,
		_main_camera_subpass_combine_ui,
		_main_camera_subpass_count
	};


	class RenderPass : public RenderPassBase
	{
	public:
		struct FrameBufferAttachment
		{
			vk::Image			image;
			vk::DeviceMemory	mem;
			vk::ImageView		view;
			//RHIFormat			format;
		};

		struct Framebuffer
		{
			int				width;
			int				height;
			vk::Framebuffer framebuffer;
			vk::RenderPass	renderPass;

			std::vector<FrameBufferAttachment> attachments;
		};

		struct Descriptor
		{
			vk::DescriptorSetLayout* layout;
			vk::DescriptorSet* descriptorSet;
		};

		struct RenderPipelineBase
		{
			vk::PipelineLayout* layout;
			vk::Pipeline* pipeline;
		};
		virtual void initialize(RenderPassInitInfo initInfo) {};

	};


}