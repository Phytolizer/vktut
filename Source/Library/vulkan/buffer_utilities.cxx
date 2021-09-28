#include "vktut/vulkan/buffer_utilities.hxx"

void vktut::vulkan::buffer_utilities::copy_buffer(VkBuffer src_buffer,
                                                  VkBuffer dst_buffer,
                                                  VkDeviceSize size,
                                                  VkQueue transfer_queue,
                                                  VkDevice device,
                                                  VkCommandPool command_pool)
{
  VkCommandBufferAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer command_buffer = nullptr;
  vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(command_buffer, &begin_info);
  VkBufferCopy copy_region = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer,
  };

  vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(transfer_queue);

  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}
