/*
 * Copyright (C) 2023, Inria
 * GRAPHDECO research group, https://team.inria.fr/graphdeco
 * All rights reserved.
 *
 * This software is free for non-commercial, research and evaluation use 
 * under the terms of the LICENSE.md file.
 *
 * For inquiries contact sibr@inria.fr and/or George.Drettakis@inria.fr
 */
#pragma once

# include "Config.hpp"
# include <core/renderer/RenderMaskHolder.hpp>
# include <core/scene/BasicIBRScene.hpp>
# include <core/system/SimpleTimer.hpp>
# include <core/system/Config.hpp>
# include <core/graphics/Mesh.hpp>
# include <core/view/ViewBase.hpp>
# include <core/renderer/CopyRenderer.hpp>
# include <core/renderer/PointBasedRenderer.hpp>
# include <atomic>
# include <mutex>
# include <memory>
# include <map>
# include <core/graphics/Texture.hpp>
#include <projects/remote/json.hpp>
#include <thread>
#include <boost/asio.hpp>
using json = nlohmann::json;

namespace sibr { 

	/**
	 * \class RemotePointView
	 * \brief Wrap a ULR renderer with additional parameters and information.
	 */
	class SIBR_EXP_ULR_EXPORT RemotePointView : public sibr::ViewBase
	{
		SIBR_CLASS_PTR(RemotePointView);

	public:

		void set_render_items(boost::asio::ip::tcp::socket& sock);

		RemotePointView(std::string ip, uint port);

		/** Replace the current scene.
		 *\param newScene the new scene to render */
		void setScene(const sibr::BasicIBRScene::Ptr & newScene);

		/**
		 * Perform rendering. Called by the view manager or rendering mode.
		 * \param dst The destination rendertarget.
		 * \param eye The novel viewpoint.
		 */
		void onRenderIBR(sibr::IRenderTarget& dst, const sibr::Camera& eye) override;

		/**
		 * Update the GUI.
		 */
		void onGUI() override;

		/** \return a reference to the scene */
		const std::shared_ptr<sibr::BasicIBRScene> & getScene() const { return _scene; }

		virtual ~RemotePointView() override;

		std::string sceneName()
		{
			std::lock_guard<std::mutex> lock(_renderDataMutex);
			return current_scene;
		}

	protected:

		std::string current_scene;

		struct RemoteRenderInfo
		{
			Vector2i imgResolution = Vector2i::Zero();
			float fovy = 0.0f;
			float fovx = 0.0f;
			float znear = 0.0f;
			float zfar = 0.0f;
			Matrix4f view = Matrix4f::Identity();
			Matrix4f viewProj = Matrix4f::Identity();
		};

		RemoteRenderInfo _remoteInfo;

		bool _doTrainingBool = true;
		bool _keepAlive = true;
		bool _showSfM = false;
		int _item_current = 0;
		std::vector<std::string> _renderItems;
		std::map<std::string, double> _metrics;

		float _scalingModifier = 1.0f;

		std::atomic<bool> keep_running = true;

		std::string _ip;
		int _port;

		void send_receive();

		GLuint _imageTexture;

		bool _imageResize = true;
		bool _imageDirty = true;
		uint32_t _timestampRequested = 1;
		std::atomic<uint32_t> _timestampReceived{0};

		// Controls, camera request, scene name, render items and metrics.
		std::mutex _renderDataMutex;
		// Completed frame pixels, dimensions and dirty flag.
		std::mutex _imageDataMutex;

		std::unique_ptr <std::thread> _networkThread;
		std::vector<unsigned char> _imageData;
		Vector2i _imageResolution = Vector2i::Zero();

		std::shared_ptr<sibr::BasicIBRScene> _scene; ///< The current scene.
		PointBasedRenderer::Ptr _pointbasedrenderer;
		CopyRenderer::Ptr _copyRenderer;
	};

} /*namespace sibr*/ 
